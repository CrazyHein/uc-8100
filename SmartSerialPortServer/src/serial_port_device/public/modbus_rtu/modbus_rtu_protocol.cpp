/*
 * modbus_rtu_protocol.cpp
 *
 *  Created on: Jun 23, 2022
 *      Author: r2h
 */
#include "modbus_rtu_protocol.hpp"
#include <string.h>

namespace serial_port_device
{

constexpr r2h_byte ModbusRtuProtocol::__CRC_TABLE_LO[256];
constexpr r2h_byte ModbusRtuProtocol::__CRC_TABLE_HI[256];

ModbusRtuProtocol::ModbusRtuProtocol(Port *port, pthread_mutex_t *pMutex) : __port(port), __mutex_ptr(pMutex), __internal_mutex(false)
{
	__adu = {0};
	__mutex = {0};
	pthread_mutexattr_t mattr = {0};

	if(pMutex == nullptr)
	{
		try
		{
			pthread_mutexattr_init(&mattr);
			if(pthread_mutex_init(&__mutex, &mattr) != 0)
				throw SysResourceException(SYS_OUT_OF_RESOURCE);
			pthread_mutexattr_destroy(&mattr);
			__mutex_ptr = &__mutex;
			__internal_mutex = true;
		}
		catch(SysResourceException &e)
		{
			pthread_mutexattr_destroy(&mattr);
			throw;
		}
	}
}

ModbusRtuProtocol::~ModbusRtuProtocol()
{
	if(__internal_mutex)
		pthread_mutex_destroy(&__mutex);
}

r2h_byte ModbusRtuProtocol::ReadHoldings(r2h_byte addr, r2h_uint16 offset, r2h_uint16 size, r2h_uint16* pData, r2h_int32 wtimeout, r2h_int32 rtimeout, bool isLittleEndianCPU)
{
	if(size > READ_HOLDINGS_MAX_IN_REGISTER)
		throw GenericException(MODBUS_RTU_READ_SIZE_OUT_OF_RANGE);
	try
	{
		pthread_mutex_lock(__mutex_ptr);
		__adu.slave_addr = addr;
		__adu.pdu.read_holdings_request.code = MODBUS_RTU_FUNC_CODE_T::READ_HOLDING_REGISTERS;
		if(isLittleEndianCPU)
		{
			__adu.pdu.read_holdings_request.offset_in_register = __swap_byte_order(offset);
			__adu.pdu.read_holdings_request.size_in_register = __swap_byte_order(size);
		}
		else
		{
			__adu.pdu.read_holdings_request.offset_in_register = offset;
			__adu.pdu.read_holdings_request.size_in_register = size;
		}
		__crc(__adu.raw, __READ_HOLDING_RSQ_CRC_LEN, __adu.raw + __READ_HOLDING_RSQ_CRC_LEN, __adu.raw + __READ_HOLDING_RSQ_CRC_LEN + 1);
		__port->Write(__adu.raw, __READ_HOLDINGS_RSQ_FRAME_LEN, &wtimeout);
		r2h_byte res = __recv_adu(sizeof(__adu.pdu.read_holdings_response_header) + size * sizeof(modbus_rtu_register), rtimeout);

		if(res == 0)
		{
			if(__adu.slave_addr != addr)
				throw GenericException(MODBUS_RTU_SLV_ADDR_MISMATCH);
			if(__adu.pdu.read_holdings_response_header.code != MODBUS_RTU_FUNC_CODE_T::READ_HOLDING_REGISTERS)
				throw GenericException(MODBUS_RTU_FUNC_CODE_MISMATCH);
			if(__adu.pdu.read_holdings_response_header.size_in_byte != size * sizeof(modbus_rtu_register))
				throw GenericException(MODBUS_RTU_REGISTER_CNT_MISMATCH);
			if(isLittleEndianCPU)
			{
				for(r2h_uint16 i = 0; i < size; ++i)
					pData[i] = __swap_byte_order(__adu.pdu.read_holdings_response.data[i]);
			}
			else
				memcpy(pData, __adu.pdu.read_holdings_response.data, __adu.pdu.read_holdings_response_header.size_in_byte);
		}
		pthread_mutex_unlock(__mutex_ptr);
		return res;
	}
	catch(GenericException &e)
	{
		pthread_mutex_unlock(__mutex_ptr);
		throw;
	}
}

r2h_byte ModbusRtuProtocol::WriteMultiples(r2h_byte addr, r2h_uint16 offset, r2h_uint16 size, const r2h_uint16* pData, r2h_int32 wtimeout, r2h_int32 rtimeout, bool isLittleEndianCPU)
{
	if(size > WRITE_MULTIPLES_MAX_REGISTER)
		throw GenericException(MODBUS_RTU_WRITE_SIZE_OUT_OF_RANGE);
	try
	{
		pthread_mutex_lock(__mutex_ptr);
		__adu.slave_addr = addr;
		__adu.pdu.write_multiple_request.code = MODBUS_RTU_FUNC_CODE_T::WRITE_MULTIPLE_REGISTERS;
		__adu.pdu.write_multiple_request_header.size_in_byte = size * sizeof(modbus_rtu_register);
		if(isLittleEndianCPU)
		{
			__adu.pdu.write_multiple_request_header.offset_in_register = __swap_byte_order(offset);
			__adu.pdu.write_multiple_request_header.size_in_register = __swap_byte_order(size);
			for(r2h_uint16 i = 0; i < size; ++i)
				__adu.pdu.write_multiple_request.data[i] = __swap_byte_order(pData[i]);
		}
		else
		{
			__adu.pdu.write_multiple_request_header.offset_in_register = offset;
			__adu.pdu.write_multiple_request_header.size_in_register = size;
			memcpy(__adu.pdu.write_multiple_request.data, pData, __adu.pdu.write_multiple_request_header.size_in_byte);
		}
		r2h_int32 len = sizeof(__adu.pdu.write_multiple_request_header) + 1 + __adu.pdu.write_multiple_request_header.size_in_byte;
		__crc(__adu.raw, len, __adu.raw + len, __adu.raw + len + 1);
		__port->Write(__adu.raw, len + 2, &wtimeout);
		r2h_byte res = __recv_adu(sizeof(__adu.pdu.write_multiple_response), rtimeout);

		if(res == 0)
		{
			if(__adu.slave_addr != addr)
				throw GenericException(MODBUS_RTU_SLV_ADDR_MISMATCH);
			if(__adu.pdu.write_multiple_response.code != MODBUS_RTU_FUNC_CODE_T::WRITE_MULTIPLE_REGISTERS)
				throw GenericException(MODBUS_RTU_FUNC_CODE_MISMATCH);
			if(__adu.pdu.write_multiple_response.offset_in_register != (isLittleEndianCPU ? __swap_byte_order(offset) : offset))
				throw GenericException(MODBUS_RTU_WRITE_RANGE_MISMATCH);
			if(__adu.pdu.write_multiple_response.size_in_register != (isLittleEndianCPU ? __swap_byte_order(size) : size))
				throw GenericException(MODBUS_RTU_WRITE_RANGE_MISMATCH);
		}

		pthread_mutex_unlock(__mutex_ptr);
		return res;
	}
	catch(GenericException &e)
	{
		pthread_mutex_unlock(__mutex_ptr);
		throw;
	}
}

r2h_byte ModbusRtuProtocol::ReadHoldings(r2h_byte addr, r2h_uint16 offset, r2h_uint16 size, GenericSharedMemory &sharedMem, r2h_uint16 start, r2h_int32 wtimeout, r2h_int32 rtimeout, bool isLittleEndianCPU)
{
	if(size > READ_HOLDINGS_MAX_IN_REGISTER)
		throw GenericException(MODBUS_RTU_READ_SIZE_OUT_OF_RANGE);
	try
	{
		pthread_mutex_lock(__mutex_ptr);
		__adu.slave_addr = addr;
		__adu.pdu.read_holdings_request.code = MODBUS_RTU_FUNC_CODE_T::READ_HOLDING_REGISTERS;
		if(isLittleEndianCPU)
		{
			__adu.pdu.read_holdings_request.offset_in_register = __swap_byte_order(offset);
			__adu.pdu.read_holdings_request.size_in_register = __swap_byte_order(size);
		}
		else
		{
			__adu.pdu.read_holdings_request.offset_in_register = offset;
			__adu.pdu.read_holdings_request.size_in_register = size;
		}
		__crc(__adu.raw, __READ_HOLDING_RSQ_CRC_LEN, __adu.raw + __READ_HOLDING_RSQ_CRC_LEN, __adu.raw + __READ_HOLDING_RSQ_CRC_LEN + 1);
		__port->Write(__adu.raw, __READ_HOLDINGS_RSQ_FRAME_LEN, &wtimeout);
		r2h_byte res = __recv_adu(sizeof(__adu.pdu.read_holdings_response_header) + size * sizeof(modbus_rtu_register), rtimeout);

		if(res == 0)
		{
			if(__adu.slave_addr != addr)
				throw GenericException(MODBUS_RTU_SLV_ADDR_MISMATCH);
			if(__adu.pdu.read_holdings_response_header.code != MODBUS_RTU_FUNC_CODE_T::READ_HOLDING_REGISTERS)
				throw GenericException(MODBUS_RTU_FUNC_CODE_MISMATCH);
			if(__adu.pdu.read_holdings_response_header.size_in_byte != size * sizeof(modbus_rtu_register))
				throw GenericException(MODBUS_RTU_REGISTER_CNT_MISMATCH);
			if(isLittleEndianCPU)
			{
				for(r2h_uint16 i = 0; i < size; ++i)
					__temp[i] = __swap_byte_order(__adu.pdu.read_holdings_response.data[i]);
				sharedMem.Write(start, __adu.pdu.read_holdings_response_header.size_in_byte, (r2h_byte*)__temp);
			}
			else
				sharedMem.Write(start, __adu.pdu.read_holdings_response_header.size_in_byte, (r2h_byte*)__adu.pdu.read_holdings_response.data);
				//memcpy(pData, __adu.pdu.read_holdings_response.data, __adu.pdu.read_holdings_response_header.size_in_byte);
		}
		pthread_mutex_unlock(__mutex_ptr);
		return res;
	}
	catch(GenericException &e)
	{
		pthread_mutex_unlock(__mutex_ptr);
		throw;
	}
}

r2h_byte ModbusRtuProtocol::WriteMultiples(r2h_byte addr, r2h_uint16 offset, r2h_uint16 size, const GenericSharedMemory &sharedMem, r2h_uint16 start, r2h_int32 wtimeout, r2h_int32 rtimeout, bool isLittleEndianCPU)
{
	if(size > WRITE_MULTIPLES_MAX_REGISTER)
		throw GenericException(MODBUS_RTU_WRITE_SIZE_OUT_OF_RANGE);
	try
	{
		pthread_mutex_lock(__mutex_ptr);
		__adu.slave_addr = addr;
		__adu.pdu.write_multiple_request.code = MODBUS_RTU_FUNC_CODE_T::WRITE_MULTIPLE_REGISTERS;
		__adu.pdu.write_multiple_request_header.size_in_byte = size * sizeof(modbus_rtu_register);
		if(isLittleEndianCPU)
		{
			__adu.pdu.write_multiple_request_header.offset_in_register = __swap_byte_order(offset);
			__adu.pdu.write_multiple_request_header.size_in_register = __swap_byte_order(size);
			sharedMem.Read(start, __adu.pdu.write_multiple_request_header.size_in_byte, (r2h_byte*)__temp);
			for(r2h_uint16 i = 0; i < size; ++i)
				__adu.pdu.write_multiple_request.data[i] = __swap_byte_order(__temp[i]);
		}
		else
		{
			__adu.pdu.write_multiple_request_header.offset_in_register = offset;
			__adu.pdu.write_multiple_request_header.size_in_register = size;
			sharedMem.Read(start, __adu.pdu.write_multiple_request_header.size_in_byte, (r2h_byte*)__adu.pdu.write_multiple_request.data);
			//memcpy(__adu.pdu.write_multiple_request.data, pData, __adu.pdu.write_multiple_request_header.size_in_byte);
		}
		r2h_int32 len = sizeof(__adu.pdu.write_multiple_request_header) + 1 + __adu.pdu.write_multiple_request_header.size_in_byte;
		__crc(__adu.raw, len, __adu.raw + len, __adu.raw + len + 1);
		__port->Write(__adu.raw, len + 2, &wtimeout);
		r2h_byte res = __recv_adu(sizeof(__adu.pdu.write_multiple_response), rtimeout);

		if(res == 0)
		{
			if(__adu.slave_addr != addr)
				throw GenericException(MODBUS_RTU_SLV_ADDR_MISMATCH);
			if(__adu.pdu.write_multiple_response.code != MODBUS_RTU_FUNC_CODE_T::WRITE_MULTIPLE_REGISTERS)
				throw GenericException(MODBUS_RTU_FUNC_CODE_MISMATCH);
			if(__adu.pdu.write_multiple_response.offset_in_register != (isLittleEndianCPU ? __swap_byte_order(offset) : offset))
				throw GenericException(MODBUS_RTU_WRITE_RANGE_MISMATCH);
			if(__adu.pdu.write_multiple_response.size_in_register != (isLittleEndianCPU ? __swap_byte_order(size) : size))
				throw GenericException(MODBUS_RTU_WRITE_RANGE_MISMATCH);
		}

		pthread_mutex_unlock(__mutex_ptr);
		return res;
	}
	catch(GenericException &e)
	{
		pthread_mutex_unlock(__mutex_ptr);
		throw;
	}
}

void ModbusRtuProtocol::__crc(r2h_byte *pData, r2h_uint16 length, r2h_byte* crcLo, r2h_byte* crcHi)
{
	*crcHi = 0xFF;
	*crcLo = 0xFF;
	r2h_uint16 index;
	while (length--)
	{
		index = (*crcLo) ^ *pData++ ;
		*crcLo = (*crcHi) ^ __CRC_TABLE_HI[index];
		*crcHi = __CRC_TABLE_LO[index];
	}
}

r2h_byte ModbusRtuProtocol::__recv_adu(r2h_int32 pduLen, r2h_int32 rtimeout)
{
	r2h_byte crcLo, crcHi;
	r2h_int32 r = rtimeout;
	__port->Read(__adu.raw, 2, &r);
	if((__adu.raw[1] & (r2h_byte)MODBUS_RTU_FUNC_CODE_T::EXCEPTION_MASK) != 0)
	{
		__port->Read(__adu.raw + 2, 3, &r);
		__crc(__adu.raw, __EXCEPTION_RSP_CRC_LEN, &crcLo, &crcHi);
		if(crcLo != *(__adu.raw + __EXCEPTION_RSP_CRC_LEN) || crcHi != *(__adu.raw + __EXCEPTION_RSP_CRC_LEN + 1))
			throw GenericException(MODBUS_RTU_CRC_ERROR);
		return __adu.pdu.exception_response.exception;
	}
	else
	{
		__port->Read(__adu.raw + 2, pduLen - 1 + 2, &r);
		__crc(__adu.raw, pduLen + 1, &crcLo, &crcHi);
		if(crcLo != *(__adu.raw + pduLen + 1) || crcHi != *(__adu.raw + pduLen + 2))
			throw GenericException(MODBUS_RTU_CRC_ERROR);
		return 0;
	}
}

SERIAL_PORT_DEVICE_MODEL_T ModbusRtuProtocol::CompatibleModel()
{
	return SERIAL_PORT_DEVICE_MODEL_T::GENERIC_MODBUS_RTU;
}

}
