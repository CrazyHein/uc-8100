/*
 * socket.cpp
 *
 *  Created on: Jun 24, 2022
 *      Author: r2h
 */
#include "socket.hpp"
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

namespace utility
{
r2h_socket create_socket()
{
	r2h_socket sc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sc == -1)
		throw SysDriverException(SYS_SOCKET_PORT_OPERATION_EXCEPTION);
	return sc;
}

void close_socket(r2h_socket devHandle)
{
	close(devHandle);
	/*
	int res = close(devHandle);
	if(res == -1)
		throw SysDriverException(SYS_SOCKET_PORT_OPERATION_EXCEPTION);
		*/
}

void socket_bind_listen(r2h_socket devHandle, r2h_const_string ip, r2h_tcp_port port, r2h_int32 backlog)
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if(ip != nullptr && ip[0] != 0)
		addr.sin_addr.s_addr = inet_addr(ip);
	else
		addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int res = bind(devHandle, (sockaddr*)&addr, sizeof(addr));
	if(res == 0)
		res = listen(devHandle, backlog);
	else
		throw SysDriverException(SYS_SOCKET_PORT_OPERATION_EXCEPTION);
	if(res != 0)
		throw SysDriverException(SYS_SOCKET_PORT_OPERATION_EXCEPTION);
}

void socket_connect(r2h_socket devHandle, r2h_const_string ip, r2h_tcp_port port)
{
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if(ip != nullptr && ip[0] != 0)
		addr.sin_addr.s_addr = inet_addr(ip);
	else
		throw GenericException(SYS_SOCKET_INVALID_IP_ADDR);

	int res = connect(devHandle, (sockaddr*)&addr, sizeof(addr));
	if(res != 0)
		throw SysDriverException(SYS_SOCKET_PORT_CONNECT_TIMEOUT);
}

void socket_accept(r2h_socket devHandle, r2h_string ip, r2h_tcp_port* port, r2h_socket* newdevHandle)
{
	struct sockaddr_in addr = {0};
	socklen_t size = sizeof(addr);
	*newdevHandle = accept(devHandle, (sockaddr*)&addr, &size);
	if(*newdevHandle == -1)
		throw SysDriverException(SYS_SOCKET_PORT_OPERATION_EXCEPTION);
	strcpy(ip, inet_ntoa(addr.sin_addr));
	*port = ntohs(addr.sin_port);
}

void socket_recv(r2h_socket devHandle, r2h_byte_array buffer, r2h_int32 size, r2h_int32 *timeout)
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
			r2h_int32 readonce = recv(devHandle, buffer + readsize, size - readsize, 0);
			if(readonce > 0) readsize += readonce;
			else
				throw SysDriverException(SYS_SOCKET_PORT_OPERATION_EXCEPTION);
		}
		else if(res < 0)
			throw SysDriverException(SYS_SOCKET_PORT_OPERATION_EXCEPTION);
		else
			throw GenericException(SYS_SOCKET_PORT_OPERATION_TIMEOUT);
	}
	*timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void socket_send(r2h_socket devHandle, r2h_const_byte_array buffer, r2h_int32 size, r2h_int32 *timeout)
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
			r2h_int32 writeonce = send(devHandle, buffer + writesize, size - writesize, 0);
			if(writeonce > 0) writesize += writeonce;
			else
				throw SysDriverException(SYS_SOCKET_PORT_OPERATION_EXCEPTION);
		}
		else if(res < 0)
			throw SysDriverException(SYS_SOCKET_PORT_OPERATION_EXCEPTION);
		else
			throw GenericException(SYS_SOCKET_PORT_OPERATION_TIMEOUT);
	}
	*timeout = tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void socket_clr(r2h_socket devHandle, r2h_int32 timeout)
{
	timeval tv = {0};
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;

	fd_set fdr;
	FD_ZERO(&fdr);
	FD_SET(devHandle, &fdr);

	r2h_byte data[16];
	while(true)
	{
		int res = select(1 + devHandle, &fdr, 0, 0, &tv);
		if(res > 0)
		{
			//r2h_int32 readonce = recv(devHandle, data, 16, 0);
			recv(devHandle, data, 16, 0);
			//if(readonce <= 0)
				//throw SysDriverException(SYS_SOCKET_PORT_OPERATION_EXCEPTION);
		}
		//else if(res < 0)
			//throw SysDriverException(SYS_SOCKET_PORT_OPERATION_EXCEPTION);
		else
			break;
	}
}
}
