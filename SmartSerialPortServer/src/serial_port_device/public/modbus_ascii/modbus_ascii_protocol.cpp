/*
 * modbus_ascii_protocol.cpp
 *
 *  Created on: Jul 14, 2022
 *      Author: r2h
 */
#include "modbus_ascii_protocol.hpp"

namespace serial_port_device
{
ModbusAsciiProtocol::ModbusAsciiProtocol(Port *port, pthread_mutex_t *pMutex) : __port(port), __mutex_ptr(pMutex), __internal_mutex(false)
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

ModbusAsciiProtocol::~ModbusAsciiProtocol()
{
	if(__internal_mutex)
		pthread_mutex_destroy(&__mutex);
}

r2h_byte ModbusAsciiProtocol::__recv_adu(r2h_int32 pduLen, r2h_int32 rtimeout)
{
	r2h_int32 r = rtimeout;
	r2h_byte lrc = 0;
	r2h_byte temp = 0;
	bool exception  = false;
	while(true)
	{
		__port->Read(__adu.raw, 1, &r);
		if(__adu.raw[0] == ':')
		{
			break;
		}
	}
	__port->Read(__adu.raw + 1, 4, &r);
	__char_to_bin8(__adu.pdu.exception_response.code, &temp);
	if((temp & (r2h_byte)MODBUS_RTU_FUNC_CODE_T::EXCEPTION_MASK) != 0)
	{
		pduLen = sizeof(__adu.pdu.exception_response);
		exception = true;
	}
	__port->Read(__adu.raw + 5, pduLen + 2, &r);
	if(*(__adu.raw + 5 + pduLen) == 0x0D && *(__adu.raw + 6 + pduLen) == 0x0A)
	{
		for(int i = 1; i < pduLen + 3; i += 2)
		{
			__char_to_bin8((r2h_char*)(__adu.raw + i), &temp);
			lrc += temp;
		}
		lrc = -lrc;
		__char_to_bin8((r2h_char*)(__adu.raw + pduLen + 3), &temp);
		if(lrc != temp)
			throw GenericException(MODBUS_ASC_LRC_ERROR);
	}
	else
		throw GenericException(MODBUS_ASC_CAN_NOT_FIND_TRAILER);

	if(exception)
	{
		__char_to_bin8(__adu.pdu.exception_response.exception, &temp);
		return temp;
	}
	else
		return 0;

}

SERIAL_PORT_DEVICE_MODEL_T ModbusAsciiProtocol::CompatibleModel()
{
	return SERIAL_PORT_DEVICE_MODEL_T::GENERIC_MODBUS_ASCII;
}

r2h_byte ModbusAsciiProtocol::ReadHoldings(r2h_byte addr, r2h_uint16 offset, r2h_uint16 size, GenericSharedMemory &sharedMem, r2h_uint16 start, r2h_int32 wtimeout, r2h_int32 rtimeout)
{
	r2h_byte lrc = addr;
	r2h_byte temp;
	if(size > READ_HOLDINGS_MAX_IN_REGISTER)
		throw GenericException(MODBUS_ASC_READ_SIZE_OUT_OF_RANGE);
	try
	{
		pthread_mutex_lock(__mutex_ptr);
		__adu.header = ':';
		__bin8_to_char(&addr, __adu.slave_addr);
		__func_code_char(MODBUS_RTU_FUNC_CODE_T::READ_HOLDING_REGISTERS, __adu.pdu.read_holdings_request.code);
		__bin16_to_char(&offset, __adu.pdu.read_holdings_request.offset_in_register);
		__bin16_to_char(&size, __adu.pdu.read_holdings_request.size_in_register);

		lrc += (r2h_byte)MODBUS_RTU_FUNC_CODE_T::READ_HOLDING_REGISTERS;
		lrc += (offset & 0x00FF) + ((offset & 0xFF00) >> 8);
		lrc += (size & 0x00FF) + ((size & 0xFF00) >> 8);
		lrc = -lrc;
		__bin8_to_char(&lrc, (r2h_char*)(__adu.raw + sizeof(__adu.pdu.read_holdings_request) + 3));
		*(__adu.raw + sizeof(__adu.pdu.read_holdings_request) + 5) = '\r';
		*(__adu.raw + sizeof(__adu.pdu.read_holdings_request) + 6) = '\n';

		__port->Write(__adu.raw, __READ_HOLDINGS_RSQ_FRAME_LEN, &wtimeout);
		r2h_byte res = __recv_adu(sizeof(__adu.pdu.read_holdings_response_header) + size * sizeof(modbus_asc_register), rtimeout);

		if(res == 0)
		{
			__char_to_bin8(__adu.slave_addr, &temp);
			if(temp != addr)
				throw GenericException(MODBUS_ASC_SLV_ADDR_MISMATCH);
			__char_to_bin8(__adu.pdu.read_holdings_response_header.code, &temp);
			if(temp != (r2h_byte)MODBUS_RTU_FUNC_CODE_T::READ_HOLDING_REGISTERS)
				throw GenericException(MODBUS_ASC_FUNC_CODE_MISMATCH);
			__char_to_bin8(__adu.pdu.read_holdings_response_header.size_in_byte, &temp);
			if(temp != size * sizeof(modbus_rtu_register))
				throw GenericException(MODBUS_ASC_REGISTER_CNT_MISMATCH);

			for(r2h_uint16 i = 0; i < size; ++i)
				__char_to_bin16((r2h_char*)(__adu.pdu.read_holdings_response.data + i), __temp + i);
			sharedMem.Write(start, temp, (r2h_byte*)__temp);
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

r2h_byte ModbusAsciiProtocol::WriteMultiples(r2h_byte addr, r2h_uint16 offset, r2h_uint16 size, const GenericSharedMemory &sharedMem, r2h_uint16 start, r2h_int32 wtimeout, r2h_int32 rtimeout)
{
	r2h_byte lrc = addr;
	r2h_byte temp;
	r2h_uint16 tempshort;
	if(size > WRITE_MULTIPLES_MAX_REGISTER)
		throw GenericException(MODBUS_ASC_WRITE_SIZE_OUT_OF_RANGE);
	try
	{
		pthread_mutex_lock(__mutex_ptr);
		__adu.header = ':';
		__bin8_to_char(&addr, __adu.slave_addr);
		__func_code_char(MODBUS_RTU_FUNC_CODE_T::WRITE_MULTIPLE_REGISTERS, __adu.pdu.write_multiple_request_header.code);
		__bin16_to_char(&offset, __adu.pdu.write_multiple_request_header.offset_in_register);
		__bin16_to_char(&size, __adu.pdu.write_multiple_request_header.size_in_register);

		lrc += (r2h_byte)MODBUS_RTU_FUNC_CODE_T::WRITE_MULTIPLE_REGISTERS;
		lrc += (offset & 0x00FF) + ((offset & 0xFF00) >> 8);
		lrc += (size & 0x00FF) + ((size & 0xFF00) >> 8);

		temp = size * sizeof(modbus_rtu_register);
		__bin8_to_char(&temp, __adu.pdu.write_multiple_request_header.size_in_byte);
		lrc += temp;

		sharedMem.Read(start, temp, (r2h_byte*)__temp);
		for(r2h_uint16 i = 0; i < size; ++i)
		{
			__bin16_to_char(__temp + i, (r2h_char*)(__adu.pdu.write_multiple_request.data + i));
			lrc += (__temp[i] & 0x00FF) + ((__temp[i] & 0xFF00) >> 8);
		}
		lrc = -lrc;

		int len = 3 + sizeof(__adu.pdu.write_multiple_request_header) + size * sizeof(modbus_asc_register);
		__bin8_to_char(&lrc, (r2h_char*)(__adu.raw + len));
		__adu.raw[2 + len] = 0x0D;
		__adu.raw[3 + len] = 0x0A;

		__port->Write(__adu.raw, len + 4, &wtimeout);
		r2h_byte res = __recv_adu(sizeof(__adu.pdu.write_multiple_response), rtimeout);


		if(res == 0)
		{
			__char_to_bin8(__adu.slave_addr, &temp);
			if(temp != addr)
				throw GenericException(MODBUS_ASC_SLV_ADDR_MISMATCH);
			__char_to_bin8(__adu.pdu.write_multiple_response.code, &temp);
			if(temp != (r2h_byte)MODBUS_RTU_FUNC_CODE_T::WRITE_MULTIPLE_REGISTERS)
				throw GenericException(MODBUS_ASC_FUNC_CODE_MISMATCH);
			__char_to_bin16(__adu.pdu.write_multiple_response.offset_in_register, &tempshort);
			if(tempshort != offset)
				throw GenericException(MODBUS_ASC_WRITE_RANGE_MISMATCH);
			__char_to_bin16(__adu.pdu.write_multiple_response.size_in_register, &tempshort);
			if(tempshort != size)
				throw GenericException(MODBUS_ASC_WRITE_RANGE_MISMATCH);
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

r2h_byte ModbusAsciiProtocol::ReadWriteRegisters(r2h_byte addr,
							r2h_uint16 writeoffset, r2h_uint16 writesize, const GenericSharedMemory &writeMem, r2h_uint16 writememstart,
							r2h_uint16 readoffset, r2h_uint16 readsize, GenericSharedMemory &readMem, r2h_uint16 readmemstart,
							r2h_int32 wtimeout, r2h_int32 rtimeout)
{
	r2h_byte lrc = addr;
	r2h_byte temp;
	if(writesize > RW_WRITE_REGISTERS_MAX_IN_REGISTER)
		throw GenericException(MODBUS_ASC_WRITE_SIZE_OUT_OF_RANGE);
	if(readsize > RW_READ_REGISTERS_MAX_IN_REGISTER)
		throw GenericException(MODBUS_ASC_READ_SIZE_OUT_OF_RANGE);
	try
	{
		pthread_mutex_lock(__mutex_ptr);
		__adu.header = ':';
		__bin8_to_char(&addr, __adu.slave_addr);
		__func_code_char(MODBUS_RTU_FUNC_CODE_T::READ_WRITE_REGISTERS, __adu.pdu.read_write_registers_request_header.code);
		__bin16_to_char(&readoffset, __adu.pdu.read_write_registers_request_header.read_offset_in_register);
		__bin16_to_char(&readsize, __adu.pdu.read_write_registers_request_header.read_size_in_register);
		__bin16_to_char(&writeoffset, __adu.pdu.read_write_registers_request_header.write_offset_in_register);
		__bin16_to_char(&writesize, __adu.pdu.read_write_registers_request_header.write_size_in_register);

		lrc += (r2h_byte)MODBUS_RTU_FUNC_CODE_T::READ_WRITE_REGISTERS;
		lrc += (readoffset & 0x00FF) + ((readoffset & 0xFF00) >> 8);
		lrc += (readsize & 0x00FF) + ((readsize & 0xFF00) >> 8);
		lrc += (writeoffset & 0x00FF) + ((writeoffset & 0xFF00) >> 8);
		lrc += (writesize & 0x00FF) + ((writesize & 0xFF00) >> 8);

		temp = writesize * sizeof(modbus_rtu_register);
		__bin8_to_char(&temp, __adu.pdu.read_write_registers_request_header.size_in_byte);
		lrc += temp;

		writeMem.Read(writememstart, temp, (r2h_byte*)__temp);
		for(r2h_uint16 i = 0; i < writesize; ++i)
		{
			__bin16_to_char(__temp + i, (r2h_char*)(__adu.pdu.read_write_registers_request.data + i));
			lrc += (__temp[i] & 0x00FF) + ((__temp[i] & 0xFF00) >> 8);
		}
		lrc = -lrc;

		int len = 3 + sizeof(__adu.pdu.read_write_registers_request_header) + writesize * sizeof(modbus_asc_register);
		__bin8_to_char(&lrc, (r2h_char*)(__adu.raw + len));
		__adu.raw[2 + len] = 0x0D;
		__adu.raw[3 + len] = 0x0A;

		__port->Write(__adu.raw, len + 4, &wtimeout);
		r2h_byte res = __recv_adu(sizeof(__adu.pdu.read_write_registers_response_header) + readsize * sizeof(modbus_asc_register), rtimeout);

		if(res == 0)
		{
			__char_to_bin8(__adu.slave_addr, &temp);
			if(temp != addr)
				throw GenericException(MODBUS_ASC_SLV_ADDR_MISMATCH);
			__char_to_bin8(__adu.pdu.read_write_registers_response_header.code, &temp);
			if(temp != (r2h_byte)MODBUS_RTU_FUNC_CODE_T::READ_WRITE_REGISTERS)
				throw GenericException(MODBUS_ASC_FUNC_CODE_MISMATCH);
			__char_to_bin8(__adu.pdu.read_write_registers_response_header.size_in_byte, &temp);
			if(temp != readsize * sizeof(modbus_rtu_register))
				throw GenericException(MODBUS_ASC_REGISTER_CNT_MISMATCH);

			for(r2h_uint16 i = 0; i < readsize; ++i)
				__char_to_bin16((r2h_char*)(__adu.pdu.read_write_registers_response.data + i), __temp + i);
			readMem.Write(readmemstart, temp, (r2h_byte*)__temp);
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

}
