/*
 * port_scanner.cpp
 *
 *  Created on: Jun 28, 2022
 *      Author: r2h
 */

#include <string.h>
#include <tuple>
#include <unistd.h>
#include "port_scanner.hpp"
#include "../serial_port_device/models.hpp"
#include "../serial_port_device/public/modbus_rtu/modbus_rtu_protocol.hpp"
#include "../serial_port_device/public/modbus_rtu/modbus_rtu_device.hpp"

using namespace serial_port_device;

namespace port_scanner
{
PortScanner::PortScanner(std::map<r2h_byte, Port*> *portCollection, GenericSharedMemory& diagMem, GenericSharedMemory& txMem, GenericSharedMemory& rxMem) :
		__port_collection(portCollection), __started(false), __stop_rsq(false), __num_of_slaves(0),
		__diag_mem(diagMem), __tx_mem(txMem), __rx_mem(rxMem), __tx_top(0), __rx_top(0)
{
	pthread_mutex_t* portOpMutex = nullptr;
	memset(__slaves_info, 0, sizeof(__slaves_info));
	memset(__slaves_status, 0, sizeof(__slaves_status));

	if(sizeof(port_server_diagnostic_info_t) > diagMem.Size())
		throw GenericException(PORT_SCANNER_OUT_OF_SHARED_MEMORY);
	try
	{
		__scanner_op_mutex = {0};
		int res = pthread_mutex_init(&__scanner_op_mutex, nullptr);
		if(res != 0)
			throw SysResourceException(SYS_OUT_OF_RESOURCE);

		for(auto p = portCollection->begin(); p != portCollection->end(); ++p)
		{
			portOpMutex  = new pthread_mutex_t();
			memset(portOpMutex, 0, sizeof(pthread_mutex_t));
			res = pthread_mutex_init(portOpMutex, nullptr);
			if(res != 0)
				throw SysResourceException(SYS_OUT_OF_RESOURCE);
			__port_op_mutexes[p->first]  = portOpMutex;
			portOpMutex = nullptr;

			__port_id_to_protocol.emplace(std::piecewise_construct, std::forward_as_tuple(p->first), std::forward_as_tuple());
			__port_id_to_device.emplace(std::piecewise_construct, std::forward_as_tuple(p->first), std::forward_as_tuple());
			//__port_id_to_protocol[p->first] = std::map<r2h_byte, IProtocol*>();
			//__port_id_to_device[p->first] = std::vector<IDevice*>();
		}
	}
	catch(GenericException &e)
	{
		if(portOpMutex != nullptr)
		{
			pthread_mutex_destroy(portOpMutex);
			delete portOpMutex;
		}
		__clean();
		throw;
	}
	catch(std::exception &e)
	{
		if(portOpMutex != nullptr)
		{
			pthread_mutex_destroy(portOpMutex);
			delete portOpMutex;
		}
		__clean();
		throw;
	}
}

PortScanner::~PortScanner()
{
	if(IsRunning())
		Stop();
	__clean();
}

void PortScanner::__clean()
{
	pthread_mutex_destroy(&__scanner_op_mutex);

	for(auto m = __port_op_mutexes.begin(); m != __port_op_mutexes.end(); ++m)
	{
		pthread_mutex_destroy(m->second);
		delete m->second;
	}
	__port_op_mutexes.clear();

	for(auto p = __port_id_to_device.begin(); p != __port_id_to_device.end(); ++p)
	{
		for(auto d = p->second.begin(); d != p->second.end(); ++d)
			delete *d;
		p->second.clear();
	}
	__port_id_to_device.clear();

	for(auto p = __port_id_to_protocol.begin(); p != __port_id_to_protocol.end(); ++p)
	{
		for(auto m = p->second.begin(); m != p->second.end(); ++m)
			delete m->second;
		p->second.clear();
	}
	__port_id_to_protocol.clear();


}

bool PortScanner::IsRunning()
{
	bool res = false;
	pthread_mutex_lock(&__scanner_op_mutex);
	res = __started;
	pthread_mutex_unlock(&__scanner_op_mutex);
	return res;
}

bool PortScanner::IsStopping()
{
	bool res = false;
	pthread_mutex_lock(&__scanner_op_mutex);
	res = __stop_rsq;
	pthread_mutex_unlock(&__scanner_op_mutex);
	return res;
}

r2h_int32 PortScanner::DeviceNum()
{
	r2h_int32 res = 0;
	pthread_mutex_lock(&__scanner_op_mutex);
	res = __num_of_slaves;
	pthread_mutex_unlock(&__scanner_op_mutex);
	return res;
}

void PortScanner::Start()
{
	try
	{
		pthread_mutex_lock(&__scanner_op_mutex);
		if(__started == true)
			throw GenericException(PORT_SCANNER_IS_RUNNING);
		if(__stop_rsq == true)
			throw GenericException(PORT_SCANNER_IS_STOPPING);

		memset(__slaves_status, 0, sizeof(__slaves_status));

		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&attr, SCHED_OTHER);

		__scan_tasks.reserve(__port_collection->size());
		pthread_t scanTask = 0;
		int res = 0;

		for(auto i = __port_collection->begin(); i !=  __port_collection->end(); ++i)
		{
			sched_param schedparam;
			schedparam.sched_priority = i->second->Priority();
			pthread_attr_setschedparam(&attr, &schedparam);
			auto param = new std::tuple<r2h_byte, void*>(i->first, this);
			res = pthread_create(&scanTask, &attr, __scan_routine, param);
			if(res != 0)
				throw SysResourceException(SYS_OUT_OF_RESOURCE);
			__scan_tasks.push_back(scanTask);
			scanTask = 0;
		}

		__started = true;
		pthread_mutex_unlock(&__scanner_op_mutex);
	}
	catch(GenericException &e)
	{
		__stop_rsq = true;
		for(auto t = __scan_tasks.begin(); t != __scan_tasks.end(); ++t)
		{
			pthread_mutex_unlock(&__scanner_op_mutex);
			pthread_join(*t, nullptr);
			pthread_mutex_lock(&__scanner_op_mutex);
		}
		__scan_tasks.clear();
		__stop_rsq = false;
		pthread_mutex_unlock(&__scanner_op_mutex);
		throw;
	}
}

void PortScanner::Stop()
{
	try
	{
		pthread_mutex_lock(&__scanner_op_mutex);
		if(__started == false)
			throw GenericException(PORT_SCANNER_NOT_RUNNING);
		if(__stop_rsq == true)
			throw GenericException(PORT_SCANNER_IS_STOPPING);

		__stop_rsq = true;

		for(auto t = __scan_tasks.begin(); t != __scan_tasks.end(); ++t)
		{
			pthread_mutex_unlock(&__scanner_op_mutex);
			pthread_join(*t, nullptr);
			pthread_mutex_lock(&__scanner_op_mutex);
		}
		__scan_tasks.clear();
		__stop_rsq = false;
		__started = false;
		pthread_mutex_unlock(&__scanner_op_mutex);
	}
	catch(GenericException &e)
	{
		pthread_mutex_unlock(&__scanner_op_mutex);
		throw;
	}
}

void PortScanner::AppendDevice(r2h_byte port, r2h_byte unit, r2h_byte model, r2h_byte instance,
					r2h_uint16 txOffset, r2h_uint16 txSize, r2h_uint16 rxOffset, r2h_uint16 rxSize,
					r2h_int32 wTimeout, r2h_int32 rTimeout, r2h_int32 prohibit)
{
	Port* p = nullptr;
	IProtocol *pt = nullptr;
	IDevice *d = nullptr;
	r2h_uint16 auctualRxSize, auctualTxSize;
	try
	{
		pthread_mutex_lock(&__scanner_op_mutex);
		if(__started == true)
			throw GenericException(PORT_SCANNER_IS_RUNNING);
		if(__stop_rsq == true)
			throw GenericException(PORT_SCANNER_IS_STOPPING);

		if(__num_of_slaves == MAX_SERIAL_PORT_DEVICE_NODES)
			throw GenericException(PORT_SCANNER_NODE_LIMIT_TRIGGERED);

		if(__port_collection->find(port) == __port_collection->end())
			throw GenericException(PORT_SCANNER_INVALID_PORT_ID);
		p = (*__port_collection)[port];
		switch(model)
		{
		case SERIAL_PORT_DEVICE_MODEL_T::GENERIC_MODBUS_RTU:
			if(txOffset % 2 != 0 || txSize % 2 != 0 || rxOffset % 2 != 0 || rxSize % 2 != 0)
				throw GenericException(PORT_SCANNER_INVALID_SLV_PARAM);
			if(__port_id_to_protocol[port].find(model) == __port_id_to_protocol[port].end())
			{
				pt = new ModbusRtuProtocol(p, __port_op_mutexes[port]);
				__port_id_to_protocol[port][model] = pt;
			}
			else
				pt = __port_id_to_protocol[port][model];
			d = new ModbusRtuDevice((ModbusRtuProtocol*)pt, unit, txOffset/2, txSize/2, rxOffset/2, rxSize/2, wTimeout, rTimeout, prohibit);
			pt = nullptr;
			auctualTxSize = txSize;
			auctualRxSize = rxSize;
			break;
		default:
			throw GenericException(PORT_SCANNER_INVALID_DEV_MODEL);
			break;
		}

		if((r2h_int32)(__tx_top + auctualTxSize) > (r2h_int32)__tx_mem.Size() || (r2h_int32)(__rx_top + auctualRxSize) > (r2h_int32)__rx_mem.Size())
		{
			throw GenericException(PORT_SCANNER_OUT_OF_SHARED_MEMORY);
		}
		d->WriteConfigure(instance, __num_of_slaves, __tx_top, __rx_top);
		__port_id_to_device[port].push_back(d);
		d = nullptr;

		__slaves_info[__num_of_slaves].port_id = port;
		__slaves_info[__num_of_slaves].unit_id = unit;
		__slaves_info[__num_of_slaves].model = model;
		__slaves_info[__num_of_slaves].instance = instance;
		__slaves_info[__num_of_slaves].tx_start = __tx_top;
		__slaves_info[__num_of_slaves].tx_size = auctualTxSize;
		__slaves_info[__num_of_slaves].rx_start = __rx_top;
		__slaves_info[__num_of_slaves].rx_size = auctualRxSize;
		__slaves_info[__num_of_slaves].wtimeout = wTimeout;
		__slaves_info[__num_of_slaves].rtimeout = rTimeout;
		__slaves_info[__num_of_slaves].prohibit = prohibit;

		__diag_mem.Write(__SLAVE_INFO_POS + __num_of_slaves * sizeof(serial_port_device_info_t),
				sizeof(serial_port_device_info_t),
				(r2h_byte*)(__slaves_info + __num_of_slaves));

		__num_of_slaves++;
		__tx_top += auctualTxSize;
		__rx_top += auctualRxSize;

		__diag_mem.Write(__SLAVE_NUM_POS, 1, &__num_of_slaves);

		pthread_mutex_unlock(&__scanner_op_mutex);
	}
	catch(GenericException &e)
	{
		if(pt != nullptr) delete pt;
		if(d != nullptr) delete d;
		pthread_mutex_unlock(&__scanner_op_mutex);
		throw;
	}
	catch(std::exception &e)
	{
		if(pt != nullptr) delete pt;
		if(d != nullptr) delete d;
		pthread_mutex_unlock(&__scanner_op_mutex);
		throw GenericException(e);
	}
}

void PortScanner::ClearDevice()
{
	try
	{
		pthread_mutex_lock(&__scanner_op_mutex);
		if(__started == true)
			throw GenericException(PORT_SCANNER_IS_RUNNING);
		if(__stop_rsq == true)
			throw GenericException(PORT_SCANNER_IS_STOPPING);

		for(auto p = __port_id_to_device.begin(); p != __port_id_to_device.end(); ++p)
		{
			for(auto d = p->second.begin(); d != p->second.end(); ++d)
				delete *d;
			p->second.clear();
		}

		for(auto p = __port_id_to_protocol.begin(); p != __port_id_to_protocol.end(); ++p)
		{
			for(auto m = p->second.begin(); m != p->second.end(); ++m)
				delete m->second;
			p->second.clear();
		}

		__num_of_slaves = 0;
		__tx_top = 0;
		__rx_top = 0;
		memset(__slaves_info, 0, sizeof(__slaves_info));

		pthread_mutex_unlock(&__scanner_op_mutex);
	}
	catch(GenericException &e)
	{
		pthread_mutex_unlock(&__scanner_op_mutex);
		throw;
	}
}

void* PortScanner::__scan_routine(void* p)
{
	std::tuple<r2h_byte, void*> *param = (std::tuple<r2h_byte, void*>*)p;
	r2h_byte port = std::get<0>(*param);
	PortScanner *pHost = (PortScanner *)std::get<1>(*param);
	delete param;

	std::vector<IDevice*> *pDevices = &pHost->__port_id_to_device[port];
	Port* pPort = (*(pHost->__port_collection))[port];
	while(true)
	{
		if(pthread_mutex_trylock(&pHost->__scanner_op_mutex) == 0)
		{
			if(pHost->__stop_rsq)
			{
				pthread_mutex_unlock(&pHost->__scanner_op_mutex);
				break;
			}
			pthread_mutex_unlock(&pHost->__scanner_op_mutex);
		}

		if(pDevices->size() == 0)
			break;

		r2h_byte dev = 0;
		timespec time;
		timespec time2;
		for(auto d = pDevices->begin(); d != pDevices->end(); ++d)
		{
			(*d)->ReadConfiguration(nullptr, &dev, nullptr, nullptr);
			clock_gettime(CLOCK_REALTIME, &time);

			pHost->__slaves_status[dev].ulast_access = time.tv_nsec/1000 + time.tv_sec * 1000000;
			pHost->__slaves_status[dev].exception = (*d)->ExchangeDataWithDevice((pHost->__tx_mem), (pHost->__rx_mem),
					pHost->__slaves_status[dev].ulast_access, &(pHost->__slaves_status[dev].uacess_interval),
					&(pHost->__slaves_status[dev].uacess_interval_max),
					&(pHost->__slaves_status[dev].uacess_interval_min));
			clock_gettime(CLOCK_REALTIME, &time2);

			pHost->__diag_mem.Write(__SLAVE_STATUS_POS + dev * sizeof(serial_port_device_status_t),
					sizeof(serial_port_device_status_t),
					(r2h_byte*)(pHost->__slaves_status + dev));

			if(pHost->__slaves_status[dev].exception == (r2h_uint16)DEVICE_EXCEPTION_CODE_T::COMMUNICATION_ERROR)
			{
				usleep(100000);//100ms
				pPort->Discard(UART_QUEUE_SELECTOR_T::IN_OUT);
			}
		}
	}
	return nullptr;
}
}
