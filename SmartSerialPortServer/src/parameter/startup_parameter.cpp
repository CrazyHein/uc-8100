/*
 * startup_parameter.cpp
 *
 *  Created on: Jul 3, 2022
 *      Author: r2h
 */

#include "startup_parameter.hpp"
#include <string.h>

namespace parameter
{
const r2h_char* port_configuration::default_prefix = "ttyM";

StartupParameter::StartupParameter(r2h_const_string path, r2h_const_string protocol): __tx_size(0), __rx_size(0), __server()
{
	xml_parse_result result = __doc.load_file(path);
	if (result.status != status_ok)
		throw GenericException(INVALID_XML_CONFIGURATION_FILE);

	r2h_char* prefix = nullptr;
	r2h_int32 temp = 0;
	try
	{
		xpath_node node = __doc.select_node("/SerialPortServer/EthernetServer");
		if(node)
		{
			for(xml_node attr = node.node().first_child(); attr; attr = attr.next_sibling())
			{
				if(strcmp(attr.name(), "IP") == 0)
					strncpy(__server.ip, attr.first_child().value(), R2H_IPv4_STR_SIZE - 1);
				else if(strcmp(attr.name(), "Port") == 0)
				{
					temp = attr.text().as_uint(-1);
					if(temp > 65535 || temp == -1) throw GenericException(INVALID_ETHSERVER_PORT);
					__server.port = temp;
				}
				else if(strcmp(attr.name(), "Concurrent") == 0)
				{
					__server.concurrent = attr.text().as_int(-1);
					if(__server.concurrent <= 0)
						throw GenericException(INVALID_ETHSERVER_CONCURRENT);
				}
				else if(strcmp(attr.name(), "WorkPriority") == 0)
				{
					__server.work_priority = attr.text().as_int(-1);
					if(__server.work_priority < 0)
						throw GenericException(INVALID_ETHSERVER_WKPRIORITY);
				}
				else if(strcmp(attr.name(), "ListenPriority") == 0)
				{
					__server.listen_priority = attr.text().as_int(-1);
					if(__server.listen_priority < 0)
						throw GenericException(INVALID_ETHSERVER_LSPRIORITY);
				}
				else if(strcmp(attr.name(), "RecvTimeout") == 0)
				{
					__server.read_timeout = attr.text().as_int(-1);
					if(__server.read_timeout < 0)
						throw GenericException(INVALID_ETHSERVER_RECV_TIMEOUT);
				}
				else if(strcmp(attr.name(), "SendTimeout") == 0)
				{
					__server.write_timeout = attr.text().as_int(-1);
					if(__server.write_timeout < 0)
						throw GenericException(INVALID_ETHSERVER_SEND_TIMEOUT);
				}
				else if(strcmp(attr.name(), "Protocol") == 0)
				{
					if(strcmp(protocol, attr.text().as_string("")) != 0)
						throw GenericException(INVALID_ETHSERVER_PROTOCOL);
				}
			}
		}

		xpath_node_set nodes = __doc.select_nodes("/SerialPortServer/SerialPorts/Port");
		for(xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			port_configuration_t pc;
			for(xml_node attr = it->node().first_child(); attr; attr = attr.next_sibling())
			{
				if(strcmp(attr.name(), "ID") == 0)
				{
					pc.id = attr.text().as_int(-1);
					if(pc.id < 0 || pc.id > 99)
						throw GenericException(INVALID_PORT_ID);
				}
				else if(strcmp(attr.name(), "Prefix") == 0)
				{
					prefix = new r2h_char[strlen(attr.first_child().value()) + 1];
					strcpy(prefix, attr.first_child().value());
					pc.prefix = prefix;
					prefix = nullptr;
				}
				else if(strcmp(attr.name(), "Mode") == 0)
				{
					if(strcmp(attr.first_child().value(), "UART_RS485_2W") == 0)
						pc.mode = UART_MODE_T::UART_MODE_RS485_2W;
					else if(strcmp(attr.first_child().value(), "UART_MODE_RS422_RS485_4W") == 0)
						pc.mode = UART_MODE_T::UART_MODE_RS422_RS485_4W;
					else if(strcmp(attr.first_child().value(), "UART_MODE_RS232") == 0)
						pc.mode = UART_MODE_T::UART_MODE_RS232;
					else
						throw GenericException(INVALID_PORT_MODE);
				}
				else if(strcmp(attr.name(), "Parity") == 0)
				{
					if(strcmp(attr.first_child().value(), "NONE") == 0)
						pc.parity = UART_PARITY_T::MSP_PARITY_NONE;
					else if(strcmp(attr.first_child().value(), "ODD") == 0)
						pc.parity = UART_PARITY_T::MSP_PARITY_ODD;
					else if(strcmp(attr.first_child().value(), "EVEN") == 0)
						pc.parity = UART_PARITY_T::MSP_PARITY_EVEN;
					else if(strcmp(attr.first_child().value(), "SPACE") == 0)
						pc.parity = UART_PARITY_T::MSP_PARITY_SPACE;
					else if(strcmp(attr.first_child().value(), "MARK") == 0)
						pc.parity = UART_PARITY_T::MSP_PARITY_MARK;
					else
						throw GenericException(INVALID_PORT_PARITY);
				}
				else if(strcmp(attr.name(), "BaudRate") == 0)
				{
					pc.baud_rate = attr.text().as_int(-1);
					if(pc.baud_rate <= 0)
						throw GenericException(INVALID_PORT_BAUDRATE);
				}
				else if(strcmp(attr.name(), "DataBits") == 0)
				{
					pc.data_bits = attr.text().as_int(-1);
					if(pc.data_bits <= 0)
						throw GenericException(INVALID_PORT_DATABITS);
				}
				else if(strcmp(attr.name(), "StopBits") == 0)
				{
					pc.stop_bits = attr.text().as_int(-1);
					if(pc.stop_bits <= 0)
						throw GenericException(INVALID_PORT_STOPBITS);
				}
				else if(strcmp(attr.name(), "Priority") == 0)
				{
					pc.priority = attr.text().as_int(-1);
					if(pc.priority < 0)
						throw GenericException(INVALID_PORT_PRIORITY);
				}
			}
			__ports.push_back(std::move(pc));
		}

		nodes = __doc.select_nodes("/SerialPortServer/SerialDevices/Device");
		for(xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
		{
			device_configuration_t dc;
			for(xml_node attr = it->node().first_child(); attr; attr = attr.next_sibling())
			{
				if(strcmp(attr.name(), "Port") == 0)
				{
					dc.port_id = attr.text().as_int(-1);
					if(dc.port_id < 0 || dc.port_id > 99)
						throw GenericException(INVALID_PORT_ID);
				}
				else if(strcmp(attr.name(), "Unit") == 0)
				{
					temp = attr.text().as_int(-1);
					if(temp > 255 || temp < 0) throw GenericException(INVALID_DEVICE_UNIT);
					dc.unit = temp;
				}
				else if(strcmp(attr.name(), "Model") == 0)
				{
					temp = attr.text().as_int(-1);
					if(temp > 255 || temp < 0) throw GenericException(INVALID_DEVICE_MODEL);
					dc.model = temp;
				}
				else if(strcmp(attr.name(), "Instance") == 0)
				{
					temp = attr.text().as_int(-1);
					if(temp > 255 || temp < 0) throw GenericException(INVALID_DEVICE_INSTANCE);
					dc.instance = temp;
				}
				else if(strcmp(attr.name(), "ReadStart") == 0)
				{
					temp = attr.text().as_int(-1);
					if(temp > 65535 || temp < 0) throw GenericException(INVALID_DEVICE_RW_RANGE);
					dc.read_start = temp;
				}
				else if(strcmp(attr.name(), "ReadSize") == 0)
				{
					temp = attr.text().as_int(-1);
					if(temp > 65535 || temp < 0) throw GenericException(INVALID_DEVICE_RW_RANGE);
					dc.read_size = temp;
				}
				else if(strcmp(attr.name(), "WriteStart") == 0)
				{
					temp = attr.text().as_int(-1);
					if(temp > 65535 || temp < 0) throw GenericException(INVALID_DEVICE_RW_RANGE);
					dc.write_start = temp;
				}
				else if(strcmp(attr.name(), "WriteSize") == 0)
				{
					temp = attr.text().as_int(-1);
					if(temp > 65535 || temp < 0) throw GenericException(INVALID_DEVICE_RW_RANGE);
					dc.write_size = temp;
				}
				else if(strcmp(attr.name(), "RecvTimeout") == 0)
				{
					dc.recv_timeout = attr.text().as_int(-1);
					if(dc.recv_timeout < 0) throw GenericException(INVALID_DEVICE_RECV_TIMEOUT);
				}
				else if(strcmp(attr.name(), "SendTimeout") == 0)
				{
					dc.send_timeout = attr.text().as_int(-1);
					if(dc.send_timeout < 0) throw GenericException(INVALID_DEVICE_SEND_TIMEOUT);
				}
				else if(strcmp(attr.name(), "Prohibit") == 0)
				{
					dc.prohibit = attr.text().as_int(-1);
					if(dc.prohibit < 0) throw GenericException(INVALID_DEVICE_PROHIBIT);
				}
			}

			temp = dc.read_size + __tx_size;
			if(temp > 65535) throw GenericException(DEVICE_RANGE_OUT_OF_SERVER_MEMORY);
			dc.read_pos = __tx_size;
			__tx_size += dc.read_size;

			temp = dc.write_size + __rx_size;
			if(temp > 65535) throw GenericException(DEVICE_RANGE_OUT_OF_SERVER_MEMORY);
			dc.write_pos = __rx_size;
			__rx_size += dc.write_size;

			__devices.push_back(dc);
		}
	}
	catch(std::exception &e)
	{
		if(prefix != nullptr) delete[] prefix;
		__ports.clear();
		__devices.clear();
		throw GenericException(e);
	}
}

StartupParameter::~StartupParameter()
{

}


}
