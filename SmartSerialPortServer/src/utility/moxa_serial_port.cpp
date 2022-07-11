/*
 * moxa_serial_port.cpp
 *
 *  Created on: Jun 21, 2022
 *      Author: r2h
 */
#include "moxa_serial_port.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

using namespace common;

namespace utility
{
static int __MOXA_UART_LIB_INITIALIZED = 0;

r2h_serial_port_handle open_serial_port(r2h_int32 port, r2h_const_string devPrefix, UART_MODE_T mode, r2h_int32 baudRate, r2h_int32 dataBits,
		r2h_int32 stopBits, UART_PARITY_T parity)
{
	int res = 0;

	if(port < 0 || port > 99)
		throw SysDriverException(PORT_NUMBER_OUT_OF_RANGE);

	if(__MOXA_UART_LIB_INITIALIZED == 0)
	{
		res = mx_uart_init();
		if(res < 0)
			throw SysDriverException(MOXA_API_PORT_INIT_EXCEPTION);
		__MOXA_UART_LIB_INITIALIZED = 1;
	}

	res = mx_uart_open(port);
	if(res < 0)
		throw SysDriverException(MOXA_API_PORT_OPEN_EXCEPTION);

	try
	{
		res = mx_uart_set_mode(port, mode);
		if(res < 0)
			throw SysDriverException(MOXA_API_PORT_MODE_EXCEPTION);
		res = mx_uart_set_baudrate(port, baudRate);
		if(res < 0)
			throw SysDriverException(MOXA_API_PORT_BAUDRATE_EXCEPTION);
		res = mx_uart_set_databits(port, dataBits);
		if(res < 0)
			throw SysDriverException(MOXA_API_PORT_DATABITS_EXCEPTION);
		res = mx_uart_set_stopbits(port, stopBits);
		if(res < 0)
			throw SysDriverException(MOXA_API_PORT_STOPBITS_EXCEPTION);
		res = mx_uart_set_parity(port, parity);
		if(res < 0)
			throw SysDriverException(MOXA_API_PORT_PARITY_EXCEPTION);
	}
	catch(SysDriverException&)
	{
		mx_uart_close(port);
		throw;
	}
	char path[16] = {0};
	sprintf(path, "/dev/%s%d", devPrefix, port);
	res = open(path, O_RDWR|O_NOCTTY|O_NDELAY);

	if(res < 0)
		throw SysDriverException(SYS_SERIAL_PORT_OPERATION_EXCEPTION);
	return res;
}

void close_serial_port(r2h_serial_port_handle devHandle)
{
	close(devHandle);
	/*
	int res = close(devHandle);
	if(res < 0)
		throw SysDriverException(SYS_SERIAL_PORT_OPERATION_EXCEPTION);
		*/
}

void write_serial_port(r2h_serial_port_handle devHandle, r2h_const_byte_array buffer, r2h_int32 size, r2h_int32* timeout)
{
	timeval tv = {0};
	tv.tv_sec = (*timeout) / 1000;
	tv.tv_usec = ((*timeout) % 1000) * 1000;

	fd_set fdw;
	FD_ZERO(&fdw);
	FD_SET(devHandle, &fdw);

	r2h_int32 writesize = 0;
	while(writesize < size)
	{
		int res = select(1 + devHandle, 0, &fdw, 0, &tv);
		if(res > 0)
		{
			r2h_int32 writeonce = write(devHandle, buffer + writesize, size - writesize);
			if(writeonce > 0) writesize += writeonce;
			else
				throw SysDriverException(SYS_SERIAL_PORT_OPERATION_EXCEPTION);
		}
		else if(res < 0)
			throw SysDriverException(SYS_SERIAL_PORT_OPERATION_EXCEPTION);
		else
			throw GenericException(SYS_SERIAL_PORT_OPERATION_TIMEOUT);
	}
	*timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void read_serial_port(r2h_serial_port_handle devHandle, r2h_byte_array buffer, r2h_int32 size, r2h_int32* timeout)
{
	timeval tv = {0};
	tv.tv_sec = (*timeout) / 1000;
	tv.tv_usec = ((*timeout) % 1000) * 1000;

	fd_set fdr;
	FD_ZERO(&fdr);
	FD_SET(devHandle, &fdr);

	r2h_int32 readsize = 0;
	while(readsize < size)
	{
		int res = select(1 + devHandle, &fdr, 0, 0, &tv);
		if(res > 0)
		{
			r2h_int32 readonce = read(devHandle, buffer + readsize, size - readsize);
			if(readonce > 0) readsize += readonce;
			else
				throw SysDriverException(SYS_SERIAL_PORT_OPERATION_EXCEPTION);
		}
		else if(res < 0)
			throw SysDriverException(SYS_SERIAL_PORT_OPERATION_EXCEPTION);
		else
			throw GenericException(SYS_SERIAL_PORT_OPERATION_TIMEOUT);
	}
	*timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void discard_serial_port(r2h_serial_port_handle devHandle, UART_QUEUE_SELECTOR_T selector)
{
	int res = 0;
	switch(selector)
	{
	case IN:
		res = tcflush(devHandle, TCIFLUSH);
		break;
	case OUT:
		res = tcflush(devHandle, TCOFLUSH);
		break;
	case IN_OUT:
		res = tcflush(devHandle, TCIOFLUSH);
		break;
	}
	if(res < 0)
		throw SysDriverException(SYS_SERIAL_PORT_OPERATION_EXCEPTION);
}

}
