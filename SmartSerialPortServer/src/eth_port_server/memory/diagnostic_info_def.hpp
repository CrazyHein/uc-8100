/*
 * diagnostic_info_def.hpp
 *
 *  Created on: Jun 27, 2022
 *      Author: r2h
 */

#ifndef ETH_PORT_SERVER_MEMORY_DIAGNOSTIC_INFO_DEF_HPP_
#define ETH_PORT_SERVER_MEMORY_DIAGNOSTIC_INFO_DEF_HPP_

#include "../../common/data_type.hpp"

using namespace common;

namespace ethernet_port_server
{

#define MAX_SERIAL_PORT_DEVICE_NODES (32)
#define SERVER_COOKIE_LENGTH (3)
#define SERVER_NAME_LENGTH (16)

typedef struct serial_port_device_info
{
	r2h_byte						port_id;
	r2h_byte						unit_id;
	r2h_byte						model;
	r2h_byte						instance;
	r2h_uint16						rx_start;
	r2h_uint16						rx_size;
	r2h_uint16						tx_start;
	r2h_uint16						tx_size;
	r2h_int32						wtimeout;
	r2h_int32						rtimeout;
	r2h_int32						prohibit;
}__attribute__((packed)) serial_port_device_info_t;

typedef struct serial_port_device_status
{
	r2h_uint16						exception;
	r2h_uint32						ulast_access;
	r2h_uint32						uacess_interval;
	r2h_uint32						uacess_interval_max;
	r2h_uint32						uacess_interval_min;
}__attribute__((packed)) serial_port_device_status_t;

typedef struct port_server_diagnostic_info
{
	r2h_char						cookie[SERVER_COOKIE_LENGTH];
	r2h_char						server_name[SERVER_NAME_LENGTH];
	r2h_byte						version;
	r2h_byte						revision;
	r2h_byte						number_of_serial_port_devices;
	serial_port_device_info_t		serial_devices_info[MAX_SERIAL_PORT_DEVICE_NODES];

	r2h_uint16						server_status;
	serial_port_device_status_t		serial_devices_status[MAX_SERIAL_PORT_DEVICE_NODES];
}__attribute__((packed)) port_server_diagnostic_info_t;

}


#endif /* ETH_PORT_SERVER_MEMORY_DIAGNOSTIC_INFO_DEF_HPP_ */
