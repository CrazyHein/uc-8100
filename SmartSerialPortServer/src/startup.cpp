//============================================================================
// Name        : SmartSerialPortServer.cpp
// Author      : r2h
// Version     :
// Copyright   : AMEC
// Description : Hello World in C++, Ansi-style
//============================================================================

#define __R2H_DEBUG

#include <iostream>
#include <unistd.h>
#include <map>
#include <signal.h>
#include "serial_port_device/public/modbus_rtu/modbus_rtu_protocol.hpp"
#include "serial_port_device/public/modbus_ascii/modbus_ascii_protocol.hpp"
#include "eth_port_server/service/sde_server.hpp"
#include "eth_port_server/memory/diagnostic_info_def.hpp"
#include "port_scanner/port_scanner.hpp"
#include "utility/moxa_led.hpp"
#include "parameter/startup_parameter.hpp"

using namespace std;
using namespace utility;
using namespace serial_port_device;
using namespace ethernet_port_server;
using namespace port_scanner;
using namespace parameter;

int main(int argc,char* argv[]) {
	std::map<r2h_byte, Port*> ports;
	Port* port = nullptr;
	StartupParameter *param = nullptr;
	GenericSharedMemory *diagMem = nullptr, *devTxMem = nullptr, *devRxMem = nullptr;
	SimpleDataExchangerServer* pServer = nullptr;
	PortScanner* pScanner = nullptr;
	try
	{
		set_led_all_off();
		if(argc != 2)
		{
			set_uc8100_red_led(LED_STATE_T::LED_STATE_BLINK);
			return -1;
		}
		set_uc8100_green_led(LED_STATE_T::LED_STATE_BLINK);

		signal(SIGPIPE,SIG_IGN);
		param = new StartupParameter(argv[1], "SDE");
		const server_configuration_t& serverParam = param->Server();
		const std::vector<port_configuration_t>& portParam = param->Ports();
		const std::vector<device_configuration_t>& deviceParam = param->Devices();

		for(auto p = portParam.begin(); p != portParam.end(); ++p)
		{
			port = new Port(p->id, p->prefix, p->mode, p->baud_rate, p->data_bits, p->stop_bits, p->parity, p->priority);
			ports[p->id] = port;
			port = nullptr;
		}

		diagMem = new GenericSharedMemory(sizeof(port_server_diagnostic_info_t), nullptr);
		devTxMem = new GenericSharedMemory(param->ExpectedTxSize(), nullptr);
		devRxMem = new GenericSharedMemory(param->ExpectedRxSize(), nullptr);

		pServer = new SimpleDataExchangerServer(nullptr, serverParam.port, serverParam.concurrent, serverParam.write_timeout, serverParam.read_timeout,
				serverParam.listen_priority, serverParam.work_priority, *diagMem, *devTxMem, *devRxMem);

		pScanner = new PortScanner(&ports, *diagMem, *devTxMem, *devRxMem);
		for(auto d = deviceParam.begin(); d != deviceParam.end(); ++d)
		{
			pScanner->AppendDevice(d->port_id, d->unit, d->model, d->instance, d->read_start, d->read_size, d->write_start, d->write_size, d->send_timeout, d->recv_timeout, d->prohibit);
		}

		pScanner->Start();
		pServer->Start();

		set_uc8100_green_led(LED_STATE_T::LED_STATE_ON);

		pthread_exit(0);
	}
	catch(std::exception &e)
	{
		if(param != nullptr)
			delete param;

		if(port != nullptr)
			delete port;
		for(auto p = ports.begin(); p != ports.end(); ++p)
			delete p->second;
		ports.clear();

		if(diagMem != nullptr)
			delete diagMem;
		if(devTxMem != nullptr)
			delete devTxMem;
		if(devRxMem != nullptr)
			delete devRxMem;

		if(pServer != nullptr)
			delete pServer;

		if(pScanner != nullptr)
			delete pScanner;

		set_led_all_off();
		set_uc8100_red_led(LED_STATE_T::LED_STATE_BLINK);
	}
	/*
	Port *p = new Port(0, "ttyM", UART_MODE_T::UART_MODE_RS485_2W, 9600, 8, 1, UART_PARITY_T::MSP_PARITY_EVEN);
	ModbusRtuProtocol protocol(p, nullptr);
	r2h_uint16 rdata[2] = {0};
	r2h_uint16 wdata[2] = {0};
	r2h_int32 res = 0;
	while(true)
	{
		try
		{
			res = protocol.ReadHoldings(1, 0, 2, rdata, 100, 100);
			if(res != 0)
				printf("Modbus RTU Exception (ReadHoldings) : 0x%02x\n", res);
			else
			{
				printf("01 : 0x%04x | 02 : 0x%04x\n", rdata[0], rdata[1]);
				wdata[0] = rdata[0];
				wdata[1] = rdata[1];
				res = protocol.WriteMultiples(1, 8, 2, wdata, 100, 100);
				if(res != 0)
					printf("Modbus RTU Exception (WriteMultiples) : 0x%02x\n", res);
			}
		}
		catch(GenericException & e)
		{
			printf("Generic Exception : 0x%08x\n", e.ErrorCode());
		}

		usleep(10000);
	}

	delete p;

	*/
}
