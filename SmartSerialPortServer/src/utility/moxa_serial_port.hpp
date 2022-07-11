/*
 * moxa_serial_port.hpp
 *
 *  Created on: Jun 21, 2022
 *      Author: r2h
 */

#ifndef UTILITY_MOXA_SERIAL_PORT_HPP_
#define UTILITY_MOXA_SERIAL_PORT_HPP_

#include <stddef.h>
#include <mx_uart.h>
#include "../common/data_type.hpp"
#include "../common/error_code_def.hpp"
#include "../common/exception.hpp"

using namespace common;

namespace utility
{

typedef enum UART_MODE {
	UART_MODE_RS232 = 0,
	UART_MODE_RS485_2W = 1,
	UART_MODE_RS422_RS485_4W = 2
}UART_MODE_T;

typedef enum UART_PARITY{
	MSP_PARITY_NONE,
	MSP_PARITY_ODD,
	MSP_PARITY_EVEN,
	MSP_PARITY_SPACE,
	MSP_PARITY_MARK
}UART_PARITY_T;

typedef enum UART_QUEUE_SELECTOR
{
	IN = 0,
	OUT = 1,
	IN_OUT = 3
}UART_QUEUE_SELECTOR_T;

r2h_serial_port_handle open_serial_port(r2h_int32 port, r2h_const_string devPrefix, UART_MODE_T mode, r2h_int32 baudRate, r2h_int32 dataBits,
		r2h_int32 stopBits, UART_PARITY_T parity);

void close_serial_port(r2h_serial_port_handle devHandle);

void write_serial_port(r2h_serial_port_handle devHandle, r2h_const_byte_array buffer, r2h_int32 size, r2h_int32* timeout);

void read_serial_port(r2h_serial_port_handle devHandle, r2h_byte_array buffer, r2h_int32 size, r2h_int32* timeout);

void discard_serial_port(r2h_serial_port_handle devHandle, UART_QUEUE_SELECTOR_T selector);
}



#endif /* UTILITY_MOXA_SERIAL_PORT_HPP_ */
