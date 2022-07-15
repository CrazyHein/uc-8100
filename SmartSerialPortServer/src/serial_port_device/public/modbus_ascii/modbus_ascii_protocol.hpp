/*
 * modbus_ascii_protocol.hpp
 *
 *  Created on: Jul 14, 2022
 *      Author: r2h
 */

#ifndef SERIAL_PORT_DEVICE_PUBLIC_MODBUS_ASCII_MODBUS_ASCII_PROTOCOL_HPP_
#define SERIAL_PORT_DEVICE_PUBLIC_MODBUS_ASCII_MODBUS_ASCII_PROTOCOL_HPP_

#include <pthread.h>
#include "../../../common/data_type.hpp"
#include "../../../common/exception.hpp"
#include "../../port.hpp"
#include "../../../eth_port_server/memory/generic_shared_memory.hpp"
#include "../../iprotocol.hpp"
#include "../modbus_rtu/modbus_rtu_protocol.hpp"

using namespace common;

namespace serial_port_device
{

#define MODBUS_ASC_ADU_SIZE_IN_BYTE (512)
#define MODBUS_ASC_PDU_SIZE_IN_BYTE (505)

typedef r2h_char modbus_asc_register[4];


typedef struct modbus_asc_pdu
{
	union
	{
		r2h_byte raw[MODBUS_ASC_PDU_SIZE_IN_BYTE];

		struct
		{
			r2h_char					code[2];
			r2h_char					offset_in_register[4];
			r2h_char					size_in_register[4];
		}__attribute__((packed)) read_holdings_request;
		struct
		{
			r2h_char					code[2];
			r2h_char					size_in_byte[2];
		}__attribute__((packed)) read_holdings_response_header;
		struct
		{
			r2h_char					code[2];
			r2h_char					size_in_byte[2];
			modbus_asc_register			data[READ_HOLDINGS_MAX_IN_REGISTER];
		}__attribute__((packed)) read_holdings_response;

		struct
		{
			r2h_char					code[2];
			r2h_char					offset_in_register[4];
			r2h_char					size_in_register[4];
			r2h_char					size_in_byte[2];
		}__attribute__((packed)) write_multiple_request_header;
		struct
		{
			r2h_char					code[2];
			r2h_char					offset_in_register[4];
			r2h_char					size_in_register[4];
			r2h_char					size_in_byte[2];
			modbus_asc_register			data[WRITE_MULTIPLES_MAX_REGISTER];
		}__attribute__((packed)) write_multiple_request;
		struct
		{
			r2h_char					code[2];
			r2h_char					offset_in_register[4];
			r2h_char					size_in_register[4];
		}__attribute__((packed)) write_multiple_response;

		struct
		{
			r2h_char					code[2];
			r2h_char					read_offset_in_register[4];
			r2h_char					read_size_in_register[4];
			r2h_char					write_offset_in_register[4];
			r2h_char					write_size_in_register[4];
			r2h_char					size_in_byte[2];
		}__attribute__((packed)) read_write_registers_request_header;
		struct
		{
			r2h_char					code[2];
			r2h_char					read_offset_in_register[4];
			r2h_char					read_size_in_register[4];
			r2h_char					write_offset_in_register[4];
			r2h_char					write_size_in_register[4];
			r2h_char					size_in_byte[2];
			modbus_asc_register			data[RW_WRITE_REGISTERS_MAX_IN_REGISTER];
		}__attribute__((packed)) read_write_registers_request;
		struct
		{
			r2h_char					code[2];
			r2h_char					size_in_byte[2];
		}__attribute__((packed)) read_write_registers_response_header;
		struct
		{
			r2h_char					code[2];
			r2h_char					size_in_byte[2];
			modbus_asc_register			data[RW_READ_REGISTERS_MAX_IN_REGISTER];
		}__attribute__((packed)) read_write_registers_response;

		struct
		{
			r2h_char					code[2];
			r2h_char					exception[2];
		}__attribute__((packed)) exception_response;
	};
}__attribute__((packed)) modbus_asc_pdu_t;

typedef struct modbus_asc_adu
{
	union
	{
		r2h_byte 						raw[MODBUS_ASC_ADU_SIZE_IN_BYTE];
		struct
		{
			r2h_char					header; // 3A
			r2h_char					slave_addr[2];
			modbus_asc_pdu_t 			pdu;
			r2h_char					lrc[2];
			r2h_char					trailer[2];
		}__attribute__((packed));
	};
}__attribute__((packed)) modbus_asc_adu_t;

class ModbusAsciiProtocol : public IProtocol
{
public:
	ModbusAsciiProtocol(Port *port, pthread_mutex_t *pMutex);
	ModbusAsciiProtocol(const ModbusRtuProtocol&) = delete;
	ModbusAsciiProtocol(ModbusRtuProtocol&&) = delete;

	virtual ~ModbusAsciiProtocol();

	r2h_byte ReadHoldings(r2h_byte addr, r2h_uint16 offset, r2h_uint16 size, GenericSharedMemory &sharedMem, r2h_uint16 start, r2h_int32 wtimeout, r2h_int32 rtimeout);
	r2h_byte WriteMultiples(r2h_byte addr, r2h_uint16 offset, r2h_uint16 size, const GenericSharedMemory &sharedMem, r2h_uint16 start, r2h_int32 wtimeout, r2h_int32 rtimeout);
	r2h_byte ReadWriteRegisters(r2h_byte addr,
								r2h_uint16 writeoffset, r2h_uint16 writesize, const GenericSharedMemory &writeMem, r2h_uint16 writememstart,
								r2h_uint16 readoffset, r2h_uint16 readsize, GenericSharedMemory &readMem, r2h_uint16 readmemstart,
								r2h_int32 wtimeout, r2h_int32 rtimeout);

	virtual SERIAL_PORT_DEVICE_MODEL_T CompatibleModel();
private:
	modbus_asc_adu_t __adu;
	modbus_rtu_register __temp[MODBUS_ASC_ADU_SIZE_IN_BYTE / sizeof(modbus_asc_register)];
	Port *__port;
	pthread_mutex_t __mutex;
	pthread_mutex_t* __mutex_ptr;
	bool __internal_mutex;

	r2h_byte __recv_adu(r2h_int32 pduLen, r2h_int32 rtimeout);

	static void __bin8_to_char(const r2h_byte *v, r2h_char* c);
	static void __bin16_to_char(const r2h_uint16 *v, r2h_char* c);
	static void __char_to_bin8(const r2h_char* c, r2h_byte *v);
	static void __char_to_bin16(const r2h_char* c, r2h_uint16 *v);
	static void __func_code_char(MODBUS_RTU_FUNC_CODE_T code, r2h_char* c);
	static void __lrc(r2h_byte *pData, r2h_uint16 length, r2h_byte* lrc);

	static constexpr r2h_int32 __READ_HOLDINGS_RSQ_FRAME_LEN = 3 + sizeof(__adu.pdu.read_holdings_request) + 4;
};

inline void ModbusAsciiProtocol::__bin8_to_char(const r2h_byte *v, r2h_char* c)
{
	r2h_byte lo = *v & 0x0F;
	r2h_byte hi = (*v & 0xF0) >> 4;
	*c = hi <= 9 ? hi + 0x30 : hi + 0x37;
	*(c + 1) = lo <= 9 ? lo + 0x30 : lo + 0x37;
}

inline void ModbusAsciiProtocol::__bin16_to_char(const r2h_uint16 *v, r2h_char* c)
{
	r2h_byte lo = *v & 0x00FF;
	r2h_byte hi = (*v & 0xFF00) >> 8;
	__bin8_to_char(&hi, c);
	__bin8_to_char(&lo, c + 2);
}

inline void ModbusAsciiProtocol::__char_to_bin8(const r2h_char* c, r2h_byte *v)
{
	r2h_byte temp = 0;
	if(*c >= 0x30 && *c <= 0x39)
		temp = (*c) - 0x30;
	else if(*c >= 0x41 && *c <= 0x46)
		temp = (*c) - 0x37;
	else
		throw GenericException(MODBUS_ASC_INVALID_CHARACTER);
	*v = temp << 4;

	if(*(c + 1) >= 0x30 && *(c + 1) <= 0x39)
		temp = (*(c + 1)) - 0x30;
	else if(*(c + 1) >= 0x41 && *(c + 1) <= 0x46)
		temp = (*(c + 1)) - 0x37;
	else
		throw GenericException(MODBUS_ASC_INVALID_CHARACTER);
	*v += temp;
}

inline void ModbusAsciiProtocol::__char_to_bin16(const r2h_char* c, r2h_uint16 *v)
{
	r2h_byte hi, lo = 0;
	__char_to_bin8(c, &hi);
	__char_to_bin8(c + 2, &lo);
	*v = (hi << 8) + lo;
}

inline void ModbusAsciiProtocol::__func_code_char(MODBUS_RTU_FUNC_CODE_T code, r2h_char* c)
{
	switch(code)
	{
	case MODBUS_RTU_FUNC_CODE_T::READ_HOLDING_REGISTERS:
		*c = 0x30;
		*(c + 1) = 0x33;
		break;
	case MODBUS_RTU_FUNC_CODE_T::WRITE_MULTIPLE_REGISTERS:
		*c = 0x31;
		*(c + 1) = 0x30;
		break;
	case MODBUS_RTU_FUNC_CODE_T::READ_WRITE_REGISTERS:
		*c = 0x31;
		*(c + 1) = 0x37;
		break;
	default:
		break;
	}
}

inline void ModbusAsciiProtocol::__lrc(r2h_byte *pData, r2h_uint16 length, r2h_byte* lrc)
{
	*lrc = 0;
	for (int i = 0; i < length; i++)
		*lrc += *pData++;
	*lrc = -*lrc;
}

}

#endif /* SERIAL_PORT_DEVICE_PUBLIC_MODBUS_ASCII_MODBUS_ASCII_PROTOCOL_HPP_ */
