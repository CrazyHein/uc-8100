/*
 * sde_server.hpp
 *
 *  Created on: Jun 25, 2022
 *      Author: r2h
 */

#ifndef ETH_PORT_SERVER_SERVICE_SDE_SERVER_HPP_
#define ETH_PORT_SERVER_SERVICE_SDE_SERVER_HPP_

#include "../memory/generic_shared_memory.hpp"
#include "../memory/diagnostic_info_def.hpp"
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <map>


namespace ethernet_port_server
{

class SimpleDataExchangerServer;

typedef struct sde_server_instance
{
	SimpleDataExchangerServer*		server;
	r2h_socket 						socket;
}sde_server_instance_t;

typedef enum class SDE_SERVER_STATUS: r2h_uint16
{
	_STOP 			= 0x0000,
	_RUNNING		= 0x0001
}SDE_SERVER_STATUS_T;

class WorkersIndicator
{
public:
	explicit WorkersIndicator(r2h_int32 index);
	WorkersIndicator(const WorkersIndicator&) = delete;
	WorkersIndicator(WorkersIndicator&&) = delete;

	virtual ~WorkersIndicator();

	void AddWorker(pthread_t id);
	void RemoveWorker(pthread_t id);
private:
	r2h_int32 __workers;
	r2h_int32 __signal_index;
	pthread_mutex_t __op_mutex;
};

class SimpleDataExchangerServer
{
public:
	SimpleDataExchangerServer(r2h_const_string ip, r2h_tcp_port port, r2h_int32 concurrent, r2h_int32 wtimeout, r2h_int32 rtimeout,
			r2h_int32 listenPriority, r2h_int32 workerPriority,
			GenericSharedMemory& diag, GenericSharedMemory& dev_tx, GenericSharedMemory& dev_rx, r2h_int32 indicatorIndex = 3);
	SimpleDataExchangerServer(const SimpleDataExchangerServer&) = delete;
	SimpleDataExchangerServer(SimpleDataExchangerServer&&) = delete;

	virtual ~SimpleDataExchangerServer();

	void Start();
	void Stop();
	bool IsRunning();

private:
	r2h_ipv4_string __binding_ip;
	r2h_tcp_port __binding_port;
	r2h_int32 __write_timeout, __read_timeout, __listen_priority, __worker_priority;
	const GenericSharedMemory &__dev_tx_mem;
	GenericSharedMemory &__diag_mem, &__dev_rx_mem;

	r2h_socket __listen_socket;
	pthread_t __listen_task;
	bool __started;
	SDE_SERVER_STATUS_T __status;

	std::vector<pthread_t> __corrupt_workers;
	std::map<pthread_t, sde_server_instance_t*> __worker_index;
	pthread_mutex_t __server_op_mutex;
	pthread_mutex_t __workers_op_mutex;
	sem_t __workers_sem_guard;

	WorkersIndicator __workers_indicator;

	void __clean();

	static r2h_byte __on_read_diag(void* host, r2h_uint16 diagStart, r2h_uint16 diagSize, r2h_byte* pDiag);
	static r2h_byte __on_read_dev_tx(void* host, r2h_uint16 devTxStart, r2h_uint16 devTxSize, r2h_byte* pDevTx);
	static r2h_byte __on_write_dev_rx(void* host, r2h_uint16 devRxStart, r2h_uint16 devRxSize, const r2h_byte* pDevRx);

	static void* __listen_routine(void *param);
	static void* __worker_routine(void *param);

	static constexpr r2h_uint16 __COOKIE_POS = (int)&(((port_server_diagnostic_info_t*)0)->cookie);
	static constexpr r2h_uint16 __NAME_POS = (int)&(((port_server_diagnostic_info_t*)0)->server_name);
	static constexpr r2h_uint16 __VERSION_POS = (int)&(((port_server_diagnostic_info_t*)0)->version);
	static constexpr r2h_uint16 __REVISION_POS = (int)&(((port_server_diagnostic_info_t*)0)->revision);
	static constexpr r2h_uint16 __STATUS_POS = (int)&(((port_server_diagnostic_info_t*)0)->server_status);
};

}

#endif /* ETH_PORT_SERVER_SERVICE_SDE_SERVER_HPP_ */
