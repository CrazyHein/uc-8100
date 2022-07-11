/*
 * port.cpp
 *
 *  Created on: Jun 22, 2022
 *      Author: r2h
 */

#include "port.hpp"

namespace serial_port_device
{
Port::Port(r2h_int32 port, r2h_const_string devPrefix, UART_MODE_T mode, r2h_int32 baudRate, r2h_int32 dataBits,
		r2h_int32 stopBits, UART_PARITY_T parity, r2h_int32 priority):__serial_port(0), __priority(priority)
{
	__serial_port = open_serial_port(port, devPrefix, mode, baudRate, dataBits, stopBits, parity);
}

Port::~Port()
{
	if(__serial_port != 0)
	{
		close_serial_port(__serial_port);
		__serial_port = 0;
	}
}

void Port::Read(r2h_byte_array buffer, r2h_int32 size, r2h_int32* timeout)
{
	read_serial_port(__serial_port, buffer, size, timeout);
}

void Port::Write(r2h_const_byte_array buffer, r2h_int32 size, r2h_int32* timeout)
{
	write_serial_port(__serial_port, buffer, size, timeout);
}

void Port::Discard(UART_QUEUE_SELECTOR_T selector)
{
	discard_serial_port(__serial_port, selector);
}

}
