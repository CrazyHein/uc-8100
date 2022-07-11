/*
 * idevice.hpp
 *
 *  Created on: Jun 22, 2022
 *      Author: r2h
 */

#ifndef SERIAL_PORT_DEVICE_IDEVICE_HPP_
#define SERIAL_PORT_DEVICE_IDEVICE_HPP_

#include "../common/data_type.hpp"
#include "../eth_port_server/memory/generic_shared_memory.hpp"
#include <pthread.h>

using namespace common;
using namespace ethernet_port_server;

namespace serial_port_device
{
class IDevice
{
public:
	IDevice():_instance(0), _dev_index(0), _production_start(0), _consume_start(0){};
	virtual void ProcessDataSize(r2h_byte instance, r2h_uint16* production, r2h_uint16* consume) = 0;
	virtual void WriteConfigure(r2h_byte instance, r2h_byte devIndex, r2h_uint16 productionStart, r2h_uint16 consumeStart) = 0;
	virtual r2h_uint16 ExchangeDataWithDevice(GenericSharedMemory &productionMem, GenericSharedMemory &consumeMem,
			r2h_uint32 uticks, r2h_uint32 *interval, r2h_uint32 *maxInterval, r2h_uint32 *minInterval) = 0;
	virtual void ReadConfiguration(r2h_byte *instance, r2h_byte *devIndex, r2h_uint16 *productionStart, r2h_uint16 *consumeStart) = 0;
	virtual ~IDevice()
	{

	}
protected:
	r2h_byte _instance, _dev_index;
	r2h_uint16 _production_start, _consume_start;
};

typedef enum class DEVICE_EXCEPTION_CODE : r2h_uint16
{
	NO_EXCEPTION						= 0x0000,
	PROTOCOL_SPECIFIC					= (0xFF00),
	COMMUNICATION_ERROR					= (0x0100),
	PORT_DRIVER_EXCEPTION				= (0x0200),
	MEMORY_ACCESS_OUT_OF_RANGE			= (0x0300)
}DEVICE_EXCEPTION_CODE_T;

}



#endif /* SERIAL_PORT_DEVICE_IDEVICE_HPP_ */
