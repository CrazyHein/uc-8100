/*
 * sde_server.cpp
 *
 *  Created on: Jun 26, 2022
 *      Author: r2h
 */
#include "sde_server.hpp"
#include "../../common/exception.hpp"
#include "../../utility/socket.hpp"
#include "../protocol/simple_data_exchanger.hpp"
#include <string.h>
#include <sys/socket.h>

#define COOKIE			("R2H")
#define NAME			("SerialPortServer")
#define VERSION			((r2h_byte)0)
#define REVISION		((r2h_byte)1)

using namespace common;
using namespace utility;

namespace ethernet_port_server
{
SimpleDataExchangerServer::SimpleDataExchangerServer(r2h_const_string ip, r2h_tcp_port port, r2h_int32 concurrent, r2h_int32 wtimeout, r2h_int32 rtimeout,
				r2h_int32 listenPriority, r2h_int32 workerPriority,
				GenericSharedMemory& diag, GenericSharedMemory& dev_tx, GenericSharedMemory& dev_rx) : __binding_port(port),
				__write_timeout(wtimeout), __read_timeout(rtimeout),
				__listen_priority(listenPriority), __worker_priority(workerPriority),
				 __dev_tx_mem(dev_tx), __diag_mem(diag), __dev_rx_mem(dev_rx),
				__listen_socket(~0), __listen_task(0), __started(false), __status(SDE_SERVER_STATUS::_STOP)
{
	try
	{
		if(ip == nullptr || strlen(ip) > sizeof(__binding_ip) - 1)
			memset(__binding_ip, 0, sizeof(__binding_ip));
		else
			strcpy(__binding_ip, ip);

		__server_op_mutex = {0};
		__workers_op_mutex = {0};
		__workers_sem_guard = {0};
		int res = pthread_mutex_init(&__server_op_mutex, nullptr);
		if(res != 0)
			throw SysResourceException(SYS_OUT_OF_RESOURCE);
		res = pthread_mutex_init(&__workers_op_mutex, nullptr);
		if(res != 0)
			throw SysResourceException(SYS_OUT_OF_RESOURCE);
		res = sem_init(&__workers_sem_guard, 0, concurrent + 1);
		if(res != 0)
			throw SysResourceException(SYS_OUT_OF_RESOURCE);

		__corrupt_workers.reserve(concurrent);

		diag.Write(__COOKIE_POS, SERVER_COOKIE_LENGTH, (r2h_byte*)COOKIE);
		diag.Write(__NAME_POS, SERVER_NAME_LENGTH, (r2h_byte*)NAME);
		r2h_byte v = VERSION;
		diag.Write(__VERSION_POS, 1, &v);
		v = REVISION;
		diag.Write(__REVISION_POS, 1, &v);
		diag.Write(__STATUS_POS, 2, (r2h_byte*)&__status);
	}
	catch(GenericException &e)
	{
		__clean();
		throw;
	}
	catch(std::exception &e)
	{
		__clean();
		throw GenericException(e);
	}
}

SimpleDataExchangerServer::~SimpleDataExchangerServer()
{
	if(IsRunning())
		Stop();
	__clean();
}

void SimpleDataExchangerServer::Start()
{
	try
	{
		pthread_mutex_lock(&__server_op_mutex);
		if(__started == true)
			throw GenericException(SDE_SERVER_IS_RUNNING);

		__listen_socket = create_socket();

		socket_bind_listen(__listen_socket, __binding_ip, __binding_port, 4);

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
		sched_param schedparam;
		schedparam.sched_priority = __listen_priority;
		pthread_attr_setschedparam(&attr, &schedparam);
		sem_wait(&__workers_sem_guard);
		int res = pthread_create(&__listen_task, &attr, __listen_routine, this);
		if(res != 0)
		{
			sem_post(&__workers_sem_guard);
			throw SysResourceException(SYS_OUT_OF_RESOURCE);
		}
		__started = true;
		__status = SDE_SERVER_STATUS::_RUNNING;
		__diag_mem.Write(__STATUS_POS, 2, (r2h_byte*)&__status);

		pthread_mutex_unlock(&__server_op_mutex);
	}
	catch(GenericException &e)
	{
		if(__listen_socket != ~0)
		{
			close_socket(__listen_socket);
			__listen_socket = ~0;
		}
		if(__listen_task != 0)
		{
			pthread_join(__listen_task, nullptr);
			__listen_task = 0;
		}
		pthread_mutex_unlock(&__server_op_mutex);
		throw;
	}
}

void SimpleDataExchangerServer::Stop()
{
	try
	{
		pthread_mutex_lock(&__server_op_mutex);
		if(__started == false)
			throw GenericException(SDE_SERVER_NOT_RUNNING);

		if(__listen_socket != ~0)
		{
			shutdown(__listen_socket, SHUT_RDWR);
			close_socket(__listen_socket);
			__listen_socket = ~0;
		}
		sem_post(&__workers_sem_guard);
		if(__listen_task != 0)
		{
			pthread_join(__listen_task, nullptr);
			__listen_task = 0;
		}
		__started = false;
		__status = SDE_SERVER_STATUS::_STOP;
		__diag_mem.Write(__STATUS_POS, 2, (r2h_byte*)&__status);
		pthread_mutex_unlock(&__server_op_mutex);
	}
	catch(GenericException &e)
	{
		pthread_mutex_unlock(&__server_op_mutex);
		throw;
	}
}

bool SimpleDataExchangerServer::IsRunning()
{
	bool res = false;
	pthread_mutex_lock(&__server_op_mutex);
	res = __started;
	pthread_mutex_unlock(&__server_op_mutex);
	return res;
}

void SimpleDataExchangerServer::__clean()
{
	memset(__binding_ip, 0, sizeof(__binding_ip));
	pthread_mutex_destroy(&__server_op_mutex);
	pthread_mutex_destroy(&__workers_op_mutex);
	sem_destroy(&__workers_sem_guard);
	__corrupt_workers.clear();
	__worker_index.clear();
}

void* SimpleDataExchangerServer::__listen_routine(void *param)
{
	SimpleDataExchangerServer* host = (SimpleDataExchangerServer*)param;
	r2h_ipv4_string ip;
	r2h_tcp_port port;
	r2h_socket socket;
	pthread_t worker;
	sde_server_instance_t* instance = nullptr;

	while(true)
	{
		sem_wait(&host->__workers_sem_guard);

		pthread_mutex_lock(&host->__workers_op_mutex);
		for(auto i = host->__corrupt_workers.begin(); i < host->__corrupt_workers.end(); ++i)
		{
			close_socket(host->__worker_index[*i]->socket);
			pthread_join(*i, nullptr);
			free(host->__worker_index[*i]);
			host->__worker_index.erase(*i);
		}
		host->__corrupt_workers.clear();
		pthread_mutex_unlock(&host->__workers_op_mutex);

		try
		{

			socket_accept(host->__listen_socket, ip, &port, &socket);

			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
			pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
			sched_param schedparam;
			schedparam.sched_priority = host->__worker_priority;
			pthread_attr_setschedparam(&attr, &schedparam);

			instance = (sde_server_instance_t*)malloc(sizeof(sde_server_instance_t));
			if(instance == nullptr)
				throw SysResourceException(SYS_OUT_OF_MEMORY);
			*instance = {host, socket};
			int res = pthread_create(&worker, &attr, __worker_routine, instance);
			if(res != 0)
				throw SysResourceException(SYS_OUT_OF_RESOURCE);
			host->__worker_index[worker] = instance;
		}
		catch(SysDriverException &e)
		{
			sem_post(&host->__workers_sem_guard);
			if(instance != nullptr) free(instance);

			/*
			//kill all workers and return
			for(auto i = host->__worker_index.begin(); i != host->__worker_index.end(); ++i)
			{
				close_socket(i->second->socket);
				pthread_join(i->first, nullptr);
				free(i->second);
			}
			host->__worker_index.clear();
			host->__corrupt_workers.clear();

			return nullptr;
			*/
		}
		catch(SysResourceException &e)
		{
			sem_post(&host->__workers_sem_guard);
			if(instance != nullptr) free(instance);
			close_socket(socket);
		}
		catch(std::exception &e)
		{
			//kill the new thread;
			if(instance != nullptr) free(instance);
			close_socket(socket);
			pthread_join(worker, nullptr);
		}
	}
}

void* SimpleDataExchangerServer::__worker_routine(void *param)
{
	sde_server_instance_t *pInstance = (sde_server_instance_t*)param;
	pthread_t id = pthread_self();
	SDEProtocol *protocol = nullptr;

	try
	{
		protocol = new SDEProtocol(pInstance->socket, nullptr);
	}
	catch(std::exception &e)
	{
		pthread_mutex_lock(&pInstance->server->__workers_op_mutex);
		pInstance->server->__corrupt_workers.push_back(id);
		pthread_mutex_unlock(&pInstance->server->__workers_op_mutex);
		return nullptr;
	}

	while(true)
	{
		try
		{
			protocol->OnSyncData(pInstance->server, __on_read_diag, __on_read_dev_tx, __on_write_dev_rx,
					pInstance->server->__write_timeout, pInstance->server->__read_timeout);
		}
		catch(SysDriverException &e)
		{
			break;
		}
		catch(GenericException &e)
		{
			if(e.ErrorCode() == (r2h_int32)SYS_SOCKET_PORT_OPERATION_TIMEOUT)
				break;
			socket_clr(pInstance->socket, 1000);
		}
	}

	delete protocol;
	pthread_mutex_lock(&pInstance->server->__workers_op_mutex);
	pInstance->server->__corrupt_workers.push_back(id);
	pthread_mutex_unlock(&pInstance->server->__workers_op_mutex);
	sem_post(&pInstance->server->__workers_sem_guard);
	return nullptr;
}

r2h_byte SimpleDataExchangerServer::__on_read_diag(void* host, r2h_uint16 diagStart, r2h_uint16 diagSize, r2h_byte* pDiag)
{
	SimpleDataExchangerServer *server = (SimpleDataExchangerServer*)host;
	try
	{
		server->__diag_mem.Read(diagStart, diagSize, pDiag);
		return (r2h_byte)SDE_EXCEPTION_CODE_T::NO_EXCEPTION;
	}
	catch(GenericException &e)
	{
		return (r2h_byte)SDE_EXCEPTION_CODE_T::READ_DIAG_OUT_OF_RANGE;
	}
}

r2h_byte SimpleDataExchangerServer::__on_read_dev_tx(void* host, r2h_uint16 devTxStart, r2h_uint16 devTxSize, r2h_byte* pDevTx)
{
	SimpleDataExchangerServer *server = (SimpleDataExchangerServer*)host;
	try
	{
		server->__dev_tx_mem.Read(devTxStart, devTxSize, pDevTx);
		return (r2h_byte)SDE_EXCEPTION_CODE_T::NO_EXCEPTION;
	}
	catch(GenericException &e)
	{
		return (r2h_byte)SDE_EXCEPTION_CODE_T::READ_DEV_TX_OUT_OF_RANGE;
	}
}

r2h_byte SimpleDataExchangerServer::__on_write_dev_rx(void* host, r2h_uint16 devRxStart, r2h_uint16 devRxSize, const r2h_byte* pDevRx)
{
	SimpleDataExchangerServer *server = (SimpleDataExchangerServer*)host;
	try
	{
		server->__dev_rx_mem.Write(devRxStart, devRxSize, pDevRx);
		return (r2h_byte)SDE_EXCEPTION_CODE_T::NO_EXCEPTION;
	}
	catch(GenericException &e)
	{
		return (r2h_byte)SDE_EXCEPTION_CODE_T::WRITE_DEV_RX_OUT_OF_RANGE;
	}
}

}
