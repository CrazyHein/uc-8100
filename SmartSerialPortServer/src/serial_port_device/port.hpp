/*
 * port.hpp
 *
 *  Created on: Jun 22, 2022
 *      Author: r2h
 */

#ifndef SERIAL_PORT_DEVICE_PORT_HPP_
#define SERIAL_PORT_DEVICE_PORT_HPP_

#include "../utility/moxa_serial_port.hpp"

using namespace utility;

namespace serial_port_device
{
class Port
{
public:
	Port(r2h_int32 port, r2h_const_string devPrefix, UART_MODE_T mode, r2h_int32 baudRate, r2h_int32 dataBits,
			r2h_int32 stopBits, UART_PARITY_T parity, r2h_int32 priority);
	Port(const Port&) = delete;
	Port(Port&&) = delete;

	void Read(r2h_byte_array buffer, r2h_int32 size, r2h_int32* timeout);
	void Write(r2h_const_byte_array buffer, r2h_int32 size, r2h_int32* timeout);
	void Discard(UART_QUEUE_SELECTOR_T selector);

	r2h_int32 Priority() const;

	virtual ~Port();
private:
	r2h_serial_port_handle __serial_port;
	r2h_int32 __priority;
};

inline r2h_int32 Port::Priority() const
{
	return __priority;
}

}

#endif /* SERIAL_PORT_DEVICE_PORT_HPP_ */
