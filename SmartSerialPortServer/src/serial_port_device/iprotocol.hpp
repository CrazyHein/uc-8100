/*
 * iprotocol.hpp
 *
 *  Created on: Jun 29, 2022
 *      Author: r2h
 */

#ifndef SERIAL_PORT_DEVICE_IPROTOCOL_HPP_
#define SERIAL_PORT_DEVICE_IPROTOCOL_HPP_

#include "../common/data_type.hpp"
#include "models.hpp"

using namespace common;

namespace serial_port_device
{
class IProtocol
{
public:
	virtual SERIAL_PORT_DEVICE_MODEL_T CompatibleModel() = 0;
	virtual ~IProtocol(){};
};
}

#endif /* SERIAL_PORT_DEVICE_IPROTOCOL_HPP_ */
