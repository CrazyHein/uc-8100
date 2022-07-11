/*
 * modbus_rtu_device.cpp
 *
 *  Created on: Jun 27, 2022
 *      Author: r2h
 */
#include <unistd.h>
#include "modbus_rtu_device.hpp"

namespace serial_port_device
{
ModbusRtuDevice::ModbusRtuDevice(ModbusRtuProtocol *protocol, r2h_uint8 unit,
		r2h_uint16 rOffset, r2h_uint16 rSize, r2h_uint16 wOffset, r2h_uint16 wSize,
		r2h_int32 wTimeout, r2h_int32 rTimeout, r2h_int32 prohibit, bool enableRW) :
				__protocol(protocol), __unit(unit),
				__roffset(rOffset), __rsize(rSize), __woffset(wOffset), __wsize(wSize),
				__wtimeout(wTimeout), __rtimeout(rTimeout), __prohibit(prohibit), __max_interval(0), __min_interval(0),
				__last_access_ticks(0), __reset(true)
{

}

ModbusRtuDevice::~ModbusRtuDevice()
{

}

void ModbusRtuDevice::ProcessDataSize(r2h_byte instance, r2h_uint16* production, r2h_uint16* consume)
{
	*production = __rsize*2;
	*consume = __wsize*2;
}

void ModbusRtuDevice::WriteConfigure(r2h_byte instance, r2h_byte devIndex, r2h_uint16 productionStart, r2h_uint16 consumeStart)
{
	_instance = instance;
	_dev_index = devIndex;
	_production_start = productionStart;
	_consume_start = consumeStart;
	__reset = true;
}

void ModbusRtuDevice::ReadConfiguration(r2h_byte *instance, r2h_byte *devIndex, r2h_uint16 *productionStart, r2h_uint16 *consumeStart)
{
	if(instance != nullptr)
		*instance = _instance;
	if(devIndex != nullptr)
		*devIndex = _dev_index;
	if(productionStart != nullptr)
		*productionStart = _production_start;
	if(consumeStart != nullptr)
		*consumeStart = _consume_start;
}

r2h_uint16 ModbusRtuDevice::ExchangeDataWithDevice(GenericSharedMemory &productionMem, GenericSharedMemory &consumeMem,
		r2h_uint32 uticks, r2h_uint32 *pInterval, r2h_uint32 *pMaxInterval, r2h_uint32 *pMinInterval)
{
	r2h_byte res = 0;
	try
	{
		if(__reset)
		{
			*pInterval = 0;
			__max_interval = 0;
			__min_interval = 0xFFFFFFFF;
			__reset = false;
		}
		else
		{
			*pInterval = uticks - __last_access_ticks;
			if(*pInterval > __max_interval) __max_interval = *pInterval;
			if(*pInterval < __min_interval) __min_interval = *pInterval;
		}
		*pMaxInterval = __max_interval;
		*pMinInterval = __min_interval;
		__last_access_ticks = uticks;

		if(*pInterval < __prohibit)
			usleep(__prohibit - *pInterval);

		res = __protocol->ReadHoldings(__unit, __roffset, __rsize, productionMem, _production_start, __wtimeout, __rtimeout);
		if(res != 0)
			return (r2h_uint16)DEVICE_EXCEPTION_CODE_T::PROTOCOL_SPECIFIC + res;

		if(__prohibit > 0)
			usleep(__prohibit);
		__protocol->WriteMultiples(__unit, __woffset, __wsize, consumeMem, _consume_start, __wtimeout, __rtimeout);
		if(res != 0)
			return (r2h_uint16)DEVICE_EXCEPTION_CODE_T::PROTOCOL_SPECIFIC + res;
		return 0;
	}
	catch(SysDriverException &e)
	{
		return (r2h_uint16)DEVICE_EXCEPTION_CODE_T::PORT_DRIVER_EXCEPTION;
	}
	catch(GenericException &e)
	{
		if(e.ErrorCode() == (r2h_int32)MEM_ACCESS_OUT_OF_RANGE)
			return (r2h_uint16)DEVICE_EXCEPTION_CODE_T::MEMORY_ACCESS_OUT_OF_RANGE;
		return (r2h_uint16)DEVICE_EXCEPTION_CODE_T::COMMUNICATION_ERROR;
	}
}
}
