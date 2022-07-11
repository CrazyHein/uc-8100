/*
 * port_sanner.hpp
 *
 *  Created on: Jun 28, 2022
 *      Author: r2h
 */

#ifndef PORT_SCANNER_PORT_SCANNER_HPP_
#define PORT_SCANNER_PORT_SCANNER_HPP_

#include <map>
#include <vector>
#include <pthread.h>
#include "../common/data_type.hpp"
#include "../serial_port_device/idevice.hpp"
#include "../serial_port_device/iprotocol.hpp"
#include "../serial_port_device/port.hpp"
#include "../eth_port_server/memory/diagnostic_info_def.hpp"

using namespace common;
using namespace serial_port_device;

namespace port_scanner
{
class PortScanner
{
public:
	explicit PortScanner(std::map<r2h_byte, Port*> *portCollection, GenericSharedMemory& diagMem, GenericSharedMemory& txMem, GenericSharedMemory& rxMem);
	PortScanner(const PortScanner&) = delete;
	PortScanner(PortScanner&&) = delete;

	virtual ~PortScanner();

	void AppendDevice(r2h_byte port, r2h_byte unit, r2h_byte model, r2h_byte instance,
			r2h_uint16 txOffset, r2h_uint16 txSize, r2h_uint16 rxOffset, r2h_uint16 rxSize,
			r2h_int32 wTimeout, r2h_int32 rTimeout, r2h_int32 prohibit);
	void ClearDevice();
	void Start();
	void Stop();
	bool IsRunning();
	bool IsStopping();
	r2h_int32 DeviceNum();

private:
	std::map<r2h_byte, Port*> *__port_collection;
	std::map<r2h_byte, pthread_mutex_t*> __port_op_mutexes;
	std::map<r2h_byte, std::map<r2h_byte, IProtocol*>> __port_id_to_protocol;
	std::map<r2h_byte, std::vector<IDevice*>> __port_id_to_device;
	std::vector<pthread_t> __scan_tasks;
	//r2h_int32 __scanner_priority;
	bool __started;
	bool __stop_rsq;
	r2h_byte __num_of_slaves;
	pthread_mutex_t __scanner_op_mutex;

	GenericSharedMemory& __diag_mem;
	GenericSharedMemory& __tx_mem;
	GenericSharedMemory& __rx_mem;
	r2h_uint16 __tx_top, __rx_top;

	serial_port_device_info_t __slaves_info[MAX_SERIAL_PORT_DEVICE_NODES];
	serial_port_device_status_t __slaves_status[MAX_SERIAL_PORT_DEVICE_NODES];

	static constexpr r2h_uint16 __SLAVE_INFO_POS = (int)&(((port_server_diagnostic_info_t*)0)->serial_devices_info);
	static constexpr r2h_uint16 __SLAVE_STATUS_POS = (int)&(((port_server_diagnostic_info_t*)0)->serial_devices_status);
	static constexpr r2h_uint16 __SLAVE_NUM_POS = (int)&(((port_server_diagnostic_info_t*)0)->number_of_serial_port_devices);

	void __clean();
	static void* __scan_routine(void*);

};

}
#endif /* PORT_SCANNER_PORT_SCANNER_HPP_ */
