/*
 * socket.hpp
 *
 *  Created on: Jun 24, 2022
 *      Author: r2h
 */

#ifndef UTILITY_SOCKET_HPP_
#define UTILITY_SOCKET_HPP_

#include "../common/data_type.hpp"
#include "../common/error_code_def.hpp"
#include "../common/exception.hpp"

using namespace common;

namespace utility
{
r2h_socket create_socket();

void close_socket(r2h_socket devHandle);

void socket_bind_listen(r2h_socket devHandle, r2h_const_string ip, r2h_tcp_port port, r2h_int32 backlog);

void socket_connect(r2h_socket devHandle, r2h_const_string ip, r2h_tcp_port port);

void socket_accept(r2h_socket devHandle, r2h_string ip, r2h_tcp_port* port, r2h_socket* newdevHandle);

void socket_recv(r2h_socket devHandle, r2h_byte_array buffer, r2h_int32 size, r2h_int32 *timeout);

void socket_send(r2h_socket devHandle, r2h_const_byte_array buffer, r2h_int32 size, r2h_int32 *timeout);

void socket_clr(r2h_socket devHandle, r2h_int32 timeout);
}



#endif /* UTILITY_SOCKET_HPP_ */
