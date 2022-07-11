/*
 * startup_parameter.hpp
 *
 *  Created on: Jul 3, 2022
 *      Author: r2h
 */

#ifndef PARAMETER_STARTUP_PARAMETER_HPP_
#define PARAMETER_STARTUP_PARAMETER_HPP_

#include <vector>
#include <string.h>

#include "../common/data_type.hpp"
#include "../common/error_code_def.hpp"
#include "../common/exception.hpp"
#include "../utility/moxa_serial_port.hpp"
#include "../utility/pugixml/pugixml.hpp"

using namespace common;
using namespace utility;
using namespace pugi;

namespace parameter
{
typedef struct server_configuration
{
	r2h_ipv4_string		ip;
	r2h_tcp_port		port;
	r2h_int32 			concurrent;
	r2h_int32			work_priority;
	r2h_int32			listen_priority;
	r2h_int32			read_timeout;
	r2h_int32			write_timeout;
	server_configuration()
	{
		strcpy(ip, "127.0.0.1");
		port = 8368;
		concurrent = 4;
		work_priority = listen_priority = 10;
		read_timeout = 60000;
		write_timeout = 1000;
	}
}server_configuration_t;

typedef struct port_configuration
{
	r2h_int32			id;
	const r2h_char*		prefix;
	r2h_byte			unit;
	UART_MODE_T			mode;
	r2h_int32			baud_rate;
	r2h_int32			data_bits;
	r2h_int32			stop_bits;
	UART_PARITY_T		parity;
	r2h_int32			priority;

	static const r2h_char* default_prefix;

	port_configuration()
	{
		id = 0;
		prefix = default_prefix;
		unit = 1;
		mode = UART_MODE_T::UART_MODE_RS232;
		baud_rate = 9600;
		data_bits = 7;
		stop_bits = 2;
		parity = UART_PARITY_T::MSP_PARITY_NONE;
		priority = 10;
	}
	port_configuration(port_configuration&& c)
	{
		id = c.id;
		prefix = c.prefix; c.prefix = nullptr;
		unit = c.unit;
		mode = c.mode;
		baud_rate = c.baud_rate;
		data_bits = c.data_bits;
		stop_bits = c.stop_bits;
		parity = c.parity;
		priority = c.priority;
	}
	port_configuration(const port_configuration&) = delete;
	~port_configuration()
	{
		if(prefix != default_prefix && prefix != nullptr)
			delete[] prefix;
	}
}port_configuration_t;

typedef struct device_configuration
{
	r2h_int32			port_id;
	r2h_byte			unit;
	r2h_byte			model;
	r2h_byte			instance;
	r2h_uint16			read_start;
	r2h_uint16			read_pos;
	r2h_uint16			read_size;
	r2h_uint16			write_start;
	r2h_uint16			write_pos;
	r2h_uint16			write_size;
	r2h_int32			recv_timeout;
	r2h_int32			send_timeout;
	r2h_int32			prohibit;
	device_configuration()
	{
		port_id = 0;
		unit = 1;
		model = 1;
		instance = 1;
		read_start = 0;
		read_size = 0;
		write_start = 0;
		write_size = 0;
		read_pos = 0;
		write_pos = 0;
		recv_timeout = 0;
		send_timeout = 0;
		prohibit = 0;
	}
}device_configuration_t;

class StartupParameter
{
public:
	explicit StartupParameter(r2h_const_string path, r2h_const_string protocol);
	StartupParameter(const StartupParameter&) = delete;
	StartupParameter(StartupParameter&&) = delete;

	virtual ~StartupParameter();

	r2h_uint16 ExpectedTxSize() const;
	r2h_uint16 ExpectedRxSize() const;
	const std::vector<port_configuration_t>& Ports() const;
	const std::vector<device_configuration_t>& Devices() const;
	const server_configuration_t& Server() const;
private:
	xml_document __doc;
	std::vector<port_configuration_t> __ports;
	std::vector<device_configuration_t> __devices;
	r2h_uint16 __tx_size, __rx_size;
	server_configuration_t __server;
};
inline r2h_uint16 StartupParameter::ExpectedTxSize() const
{
	return __tx_size;
}

inline r2h_uint16 StartupParameter::ExpectedRxSize() const
{
	return __rx_size;
}

inline const std::vector<port_configuration_t>& StartupParameter::Ports() const
{
	return __ports;
}

inline const std::vector<device_configuration_t>& StartupParameter::Devices() const
{
	return __devices;
}

inline const server_configuration_t& StartupParameter::Server() const
{
	return __server;
}

}



#endif /* PARAMETER_STARTUP_PARAMETER_HPP_ */
