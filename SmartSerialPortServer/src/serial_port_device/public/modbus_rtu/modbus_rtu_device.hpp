/*
 * modbus_rtu_device.hpp
 *
 *  Created on: Jun 22, 2022
 *      Author: r2h
 */

#ifndef SERIAL_PORT_DEVICE_PUBLIC_MODBUS_RTU_MODBUS_RTU_DEVICE_HPP_
#define SERIAL_PORT_DEVICE_PUBLIC_MODBUS_RTU_MODBUS_RTU_DEVICE_HPP_

#include "../../idevice.hpp"
#include "modbus_rtu_protocol.hpp"

namespace serial_port_device
{

class ModbusRtuDevice:public IDevice
{
public:
	ModbusRtuDevice(ModbusRtuProtocol *protocol, r2h_uint8 unit,
			r2h_uint16 rOffset, r2h_uint16 rSize, r2h_uint16 wOffset, r2h_uint16 wSize,
			r2h_int32 wTimeout, r2h_int32 rTimeout, r2h_int32 prohibit, bool enableRW = true);
	ModbusRtuDevice(const ModbusRtuDevice&) = delete;
	ModbusRtuDevice(ModbusRtuDevice&&) = delete;

	virtual ~ModbusRtuDevice();

	virtual void ProcessDataSize(r2h_byte instance, r2h_uint16* production, r2h_uint16* consume);
	virtual void WriteConfigure(r2h_byte instance, r2h_byte devIndex, r2h_uint16 productionStart, r2h_uint16 consumeStart);
	virtual r2h_uint16 ExchangeDataWithDevice(GenericSharedMemory &productionMem, GenericSharedMemory &consumeMem,
			r2h_uint32 uticks, r2h_uint32 *interval, r2h_uint32 *maxInterval, r2h_uint32 *minInterval);
	virtual void ReadConfiguration(r2h_byte *instance, r2h_byte *devIndex, r2h_uint16 *productionStart, r2h_uint16 *consumeStart);
	virtual r2h_int32 ReadTimeout();
	virtual r2h_int32 WriteTimeout();
	virtual r2h_int32 ProhibitTime();

private:
	ModbusRtuProtocol* __protocol;
	const r2h_uint8 __unit;
	const r2h_uint16 __roffset, __rsize, __woffset, __wsize;
	r2h_uint16 __last_ret;
	const r2h_int32 __wtimeout, __rtimeout;
	const r2h_uint32 __prohibit;
	r2h_uint32 __max_interval, __min_interval, __last_access_ticks, __last_op_ticks;
	bool __reset;
	bool __enable_rw;

};

inline r2h_int32 ModbusRtuDevice::ReadTimeout()
{
	return __rtimeout;
}

inline r2h_int32 ModbusRtuDevice::WriteTimeout()
{
	return __wtimeout;
}

inline r2h_int32 ModbusRtuDevice::ProhibitTime()
{
	return __prohibit;
}

}

#endif /* SERIAL_PORT_DEVICE_PUBLIC_MODBUS_RTU_MODBUS_RTU_DEVICE_HPP_ */
