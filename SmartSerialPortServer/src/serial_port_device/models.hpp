/*
 * models.hpp
 *
 *  Created on: Jun 29, 2022
 *      Author: r2h
 */

#ifndef SERIAL_PORT_DEVICE_MODELS_HPP_
#define SERIAL_PORT_DEVICE_MODELS_HPP_

#include "../common/data_type.hpp"

using namespace common;

namespace serial_port_device
{

typedef enum SERIAL_PORT_DEVICE_MODEL: r2h_byte
{
	GENERIC_MODBUS_RTU				= 0x01,
}SERIAL_PORT_DEVICE_MODEL_T;

}


#endif /* SERIAL_PORT_DEVICE_MODELS_HPP_ */
