/*
 * simple_data_exchange.hpp
 *
 *  Created on: Jun 24, 2022
 *      Author: r2h
 */

#ifndef ETH_PORT_SERVER_PROTOCOL_SIMPLE_DATA_EXCHANGER_HPP_
#define ETH_PORT_SERVER_PROTOCOL_SIMPLE_DATA_EXCHANGER_HPP_

#include <pthread.h>
#include "../../common/data_type.hpp"
#include "../../common/exception.hpp"
#include "../memory/generic_shared_memory.hpp"

using namespace common;

namespace ethernet_port_server
{

typedef struct sde_func_code
{
	union
	{
		r2h_byte 						raw_value;
		struct
		{
			r2h_byte					read_diagnostic 	: 1;
			r2h_byte					read_dev_tx_pdo 	: 1;
			r2h_byte					write_dev_rx_pdo 	: 1;
			r2h_byte					dummy_bit3			: 1;
			r2h_byte					cmd_flag		 	: 1;
			r2h_byte					ack_flag		 	: 1;
			r2h_byte					dummy_bit7 			: 1;
			r2h_byte					exception_flag 		: 1;
		};
	};
}__attribute__((packed)) sde_func_code_t;

typedef struct sde_addr
{
	r2h_uint16							start;
	r2h_uint16							size;
}__attribute__((packed)) sde_addr_t;

typedef struct sde_datagram_header
{
	r2h_uint16 							transaction;
	r2h_byte							protocol_identifier[3];//0x53 0x44 0x45
	sde_func_code_t						func;
	sde_addr_t							diag_addr;
	sde_addr_t							dev_tx_addr;
	sde_addr_t							dev_rx_addr;
	r2h_uint16							bytes;
}__attribute__((packed)) sde_datagram_header_t;

typedef enum class SDE_EXCEPTION_CODE : r2h_byte
{
	NO_EXCEPTION						= 0x00,

	WRITE_DEV_RX_UNSUPPORTED			= 0x01,
	READ_DIAG_UNSUPPORTED				= 0x02,
	READ_DEV_TX_UNSUPPORTED				= 0x03,

	READ_DIAG_OUT_OF_RANGE				= 0x10,
	READ_DEV_TX_OUT_OF_RANGE			= 0x11,
	WRITE_DEV_RX_OUT_OF_RANGE			= 0x12,

	SDE_EC_READ_SIZE_LIMIT				= 0x20,
	SDE_EC_WRITE_SIZE_LIMIT				= 0x21,
	SDE_EC_NOT_EXPECTED_MSG_SIZE		= 0x22,

}SDE_EXCEPTION_CODE_T;

typedef r2h_byte (*pfunc_on_read_diag)(void* host, r2h_uint16 diagStart, r2h_uint16 diagSize, r2h_byte* pDiag);
typedef r2h_byte (*pfunc_on_read_dev_tx)(void* host, r2h_uint16 devTxStart, r2h_uint16 devTxSize, r2h_byte* pDevTx);
typedef r2h_byte (*pfunc_on_write_dev_rx)(void* host, r2h_uint16 devRxStart, r2h_uint16 devRxSize, const r2h_byte* pDevRx);

class SDEProtocol
{
public:
	SDEProtocol(r2h_socket tcp, pthread_mutex_t *pMutex);
	SDEProtocol(const SDEProtocol&) = delete;
	SDEProtocol(SDEProtocol&&) = delete;

	virtual ~SDEProtocol();

	r2h_byte SyncData(sde_func_code_t func,
			r2h_uint16 diagStart, r2h_uint16 diagSize, r2h_byte* pDiag,
			r2h_uint16 devTxStart, r2h_uint16 devTxSize, r2h_byte* pDevTx,
			r2h_uint16 devRxStart, r2h_uint16 devRxSize, const r2h_byte* pDevRx,
			r2h_int32 wtimeout, r2h_int32 rtimeout, bool &exception);
	void OnSyncData(void* host, pfunc_on_read_diag onReadDiag, pfunc_on_read_dev_tx onReadDevTx, pfunc_on_write_dev_rx onWriteDevRx,
			r2h_int32 wtimeout, r2h_int32 rtimeout);

	static constexpr r2h_uint32 BATCH_READ_LIMIT = 65535;
	static constexpr r2h_uint32 BATCH_WRITE_LIMIT = 65535;
	static constexpr r2h_uint16 AREA_ADDR_SIZE = 3 * sizeof(sde_addr_t);
	static constexpr char PROTOCOL_IDENTIFIER[] = {'S', 'D', 'E'};
private:
	r2h_socket __tcp;
	r2h_byte __datagram[sizeof(sde_datagram_header_t) + 64 * 1024 - 1];
	pthread_mutex_t __mutex;
	pthread_mutex_t* __mutex_ptr;
	bool __internal_mutex;
	r2h_uint16 __transaction;

	r2h_byte __recv_ack_datagram(r2h_int32 rtimeout, bool &exception);
	r2h_byte __recv_cmd_datagram(r2h_int32 rtimeout);
};


}



#endif /* ETH_PORT_SERVER_PROTOCOL_SIMPLE_DATA_EXCHANGER_HPP_ */
