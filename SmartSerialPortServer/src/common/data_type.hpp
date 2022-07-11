/*
 * data_type.hpp
 *
 *  Created on: Jun 21, 2022
 *      Author: r2h
 */

#ifndef COMMON_DATA_TYPE_HPP_
#define COMMON_DATA_TYPE_HPP_

#include <stdint.h>
namespace common
{
typedef int32_t 				r2h_int32;
typedef int16_t					r2h_int16;
typedef int64_t 				r2h_int64;
typedef uint32_t 				r2h_uint32;
typedef uint16_t				r2h_uint16;
typedef uint8_t					r2h_uint8;
typedef uint8_t					r2h_byte;
typedef int8_t					r2h_sbyte;
typedef int8_t					r2h_int8;
typedef char					r2h_char;
typedef char*					r2h_string;
typedef const char*				r2h_const_string;
typedef r2h_byte*				r2h_byte_array;
typedef const r2h_byte*			r2h_const_byte_array;

typedef float					r2h_float;
typedef double					r2h_double;

typedef r2h_uint16 				r2h_tcp_port;
typedef int 					r2h_socket;

#define R2H_IPv4_STR_SIZE		(16)
typedef r2h_char 				r2h_ipv4_string[R2H_IPv4_STR_SIZE];

typedef int 					r2h_serial_port_handle;

#ifdef __R2H_DEBUG
#define __r2h_printf(arg)\
	printf(arg);\
	printf("\n");\
	fflush(stdout);
#else
#define __r2h_printf(arg)
#endif

}

#endif /* COMMON_DATA_TYPE_HPP_ */
