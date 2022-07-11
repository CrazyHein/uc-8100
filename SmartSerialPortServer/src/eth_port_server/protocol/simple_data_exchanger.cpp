/*
 * simple_data_exchanger.cpp
 *
 *  Created on: Jun 24, 2022
 *      Author: r2h
 */
#include "simple_data_exchanger.hpp"
#include "../../utility/socket.hpp"
#include <string.h>

using namespace utility;

namespace ethernet_port_server
{
constexpr char SDEProtocol::PROTOCOL_IDENTIFIER[];

SDEProtocol::SDEProtocol(r2h_socket tcp, pthread_mutex_t *pMutex) : __tcp(tcp), __mutex_ptr(pMutex), __internal_mutex(false), __transaction(0)
{
	memset(__datagram, 0, sizeof(__datagram));
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

SDEProtocol::~SDEProtocol()
{
	if(__internal_mutex)
		pthread_mutex_destroy(&__mutex);
}

r2h_byte SDEProtocol::SyncData(sde_func_code_t func,
		r2h_uint16 diagStart, r2h_uint16 diagSize, r2h_byte* pDiag,
		r2h_uint16 devTxStart, r2h_uint16 devTxSize, r2h_byte* pDevTx,
		r2h_uint16 devRxStart, r2h_uint16 devRxSize, const r2h_byte* pDevRx,
		r2h_int32 wtimeout, r2h_int32 rtimeout, bool &exception)
{
	try
	{
		pthread_mutex_lock(__mutex_ptr);
		sde_datagram_header_t *req = (sde_datagram_header_t*)__datagram;
		memset(req, 0, sizeof(sde_datagram_header_t));
		req->transaction = __transaction++;
		memcpy(req->protocol_identifier, PROTOCOL_IDENTIFIER, 3);
		req->func.cmd_flag = 1;
		req->bytes = 0;
		req->diag_addr.start = diagStart;
		req->diag_addr.size = diagSize;
		req->dev_tx_addr.start = devTxStart;
		req->dev_tx_addr.size = devTxSize;
		req->dev_rx_addr.start = devRxStart;
		req->dev_rx_addr.size = devRxSize;
		if (func.read_diagnostic == 1)
			req->func.read_diagnostic = 1;
		if (func.read_dev_tx_pdo == 1)
			req->func.read_dev_tx_pdo = 1;
		if (func.write_dev_rx_pdo == 1)
		{
			req->func.write_dev_rx_pdo = 1;
			req->bytes += devRxSize;
			memcpy(__datagram + sizeof(sde_datagram_header_t), pDevRx, devRxSize);
		}

		r2h_uint32 rLen = (req->func.read_diagnostic == 1 ? diagSize : 0) + (req->func.read_dev_tx_pdo == 1 ? devTxSize : 0);
		if (rLen > BATCH_READ_LIMIT)
			throw GenericException(SDE_READ_OUT_OF_RANGE);
		if (devRxSize > BATCH_WRITE_LIMIT)
			throw GenericException(SDE_WRITE_OUT_OF_RANGE);

		r2h_uint16 transaction = req->transaction;
		socket_send(__tcp, __datagram, sizeof(sde_datagram_header_t) + (req->func.write_dev_rx_pdo == 1 ? devRxSize : 0), &wtimeout);
		r2h_byte res = __recv_ack_datagram(rtimeout, exception);

		if(res == 0 && exception == false)
		{
			if(req->transaction != transaction)
				throw GenericException(SDE_TRANSACTION_MISMATCH);
			if(req->func.read_diagnostic != func.read_diagnostic ||
					req->func.read_dev_tx_pdo != func.read_dev_tx_pdo ||
					req->func.write_dev_rx_pdo != func.write_dev_rx_pdo)
				throw GenericException(SDE_FUNC_CODE_MISMATCH);
			if(req->diag_addr.size != diagSize || req->diag_addr.start != diagStart ||
					req->dev_tx_addr.size != devTxSize || req->dev_tx_addr.start != devTxStart ||
					req->dev_rx_addr.size != devRxSize || req->dev_rx_addr.start != devRxStart)
				throw GenericException(SDE_AREA_ADDR_MISMATCH);
			if(req->bytes != rLen)
				throw GenericException(SDE_NOT_EXPECTED_MSG_SIZE);

			memcpy(pDiag, __datagram + sizeof(sde_datagram_header_t), diagSize);
			memcpy(pDevTx, __datagram + sizeof(sde_datagram_header_t) + diagSize, devTxSize);
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

void SDEProtocol::OnSyncData(void* host, pfunc_on_read_diag onReadDiag, pfunc_on_read_dev_tx onReadDevTx, pfunc_on_write_dev_rx onWriteDevRx,
		r2h_int32 wtimeout, r2h_int32 rtimeout)
{
	try
	{
		pthread_mutex_lock(__mutex_ptr);
		__recv_cmd_datagram(rtimeout);
		sde_datagram_header_t *h = (sde_datagram_header_t*)__datagram;
		r2h_byte res = 0;

		r2h_uint16 diagLen = 0, txLen = 0;
		if(h->func.read_diagnostic == 1)
			diagLen = h->diag_addr.size;
		if(h->func.read_dev_tx_pdo == 1)
			txLen += h->dev_tx_addr.size;

		if(diagLen + txLen > BATCH_READ_LIMIT)
			//throw GenericException(SDE_READ_OUT_OF_RANGE);
			res = (r2h_byte)SDE_EXCEPTION_CODE_T::SDE_EC_READ_SIZE_LIMIT;
		if(res == 0 && h->func.write_dev_rx_pdo == 1)
		{
			if(h->dev_rx_addr.size > BATCH_WRITE_LIMIT)
				//throw GenericException(SDE_WRITE_OUT_OF_RANGE);
				res = (r2h_byte)SDE_EXCEPTION_CODE_T::SDE_EC_WRITE_SIZE_LIMIT;
			else if(h->bytes != h->dev_rx_addr.size)
				//throw GenericException(SDE_NOT_EXPECTED_DATA_SIZE);
				res = (r2h_byte)SDE_EXCEPTION_CODE_T::SDE_EC_NOT_EXPECTED_MSG_SIZE;
			else if(onWriteDevRx != nullptr)
				res = onWriteDevRx(host, h->dev_rx_addr.start, h->dev_rx_addr.size, __datagram + sizeof(sde_datagram_header_t));
			else
				res = (r2h_byte)SDE_EXCEPTION_CODE_T::WRITE_DEV_RX_UNSUPPORTED;
		}
		if(res == 0 && h->func.read_diagnostic == 1)
		{
			if(onReadDiag != nullptr)
				res = onReadDiag(host, h->diag_addr.start, h->diag_addr.size, __datagram + sizeof(sde_datagram_header_t));
			else
				res = (r2h_byte)SDE_EXCEPTION_CODE_T::READ_DIAG_UNSUPPORTED;
		}
		if(res == 0 && h->func.read_dev_tx_pdo == 1)
		{
			if(onReadDevTx != nullptr)
				res = onReadDevTx(host, h->dev_tx_addr.start, h->dev_tx_addr.size, __datagram + sizeof(sde_datagram_header_t) + diagLen);
			else
				res = (r2h_byte)SDE_EXCEPTION_CODE_T::READ_DEV_TX_UNSUPPORTED;
		}


		h->func.cmd_flag = 0;
		h->func.ack_flag = 1;
		if(res != 0)
		{
			h->func.exception_flag = 1;
			h->bytes = 1;
			__datagram[sizeof(sde_datagram_header_t)] = res;
		}
		else
		{
			h->func.exception_flag = 0;
			h->bytes = diagLen + txLen;
		}
		socket_send(__tcp, __datagram, sizeof(sde_datagram_header_t) + h->bytes, &wtimeout);


		pthread_mutex_unlock(__mutex_ptr);
	}
	catch(GenericException &e)
	{
		pthread_mutex_unlock(__mutex_ptr);
		throw;
	}
}

r2h_byte SDEProtocol::__recv_ack_datagram(r2h_int32 rtimeout, bool &exception)
{
	r2h_int32 r = rtimeout;
	sde_datagram_header_t *h = (sde_datagram_header_t*)__datagram;
	while(true)
	{
		socket_recv(__tcp, __datagram, sizeof(sde_datagram_header_t), &r);
		socket_recv(__tcp, __datagram + sizeof(sde_datagram_header_t), h->bytes, &r);
		if(h->func.ack_flag == 1)
			break;
	}
	if(strncmp((char*)(__datagram + 2), PROTOCOL_IDENTIFIER, sizeof(PROTOCOL_IDENTIFIER)) != 0)
		throw GenericException(SDE_RECV_INVALID_DATAGRAM);
	if(h->func.exception_flag == 1)
	{
		exception = true;
		if(h->bytes!= sizeof(r2h_byte))
			throw GenericException(SDE_RECV_INVALID_DATAGRAM);
		else
			return *(__datagram + sizeof(sde_datagram_header_t) + 1);
	}
	else
	{
		exception = false;
		return 0;
	}
}

r2h_byte SDEProtocol::__recv_cmd_datagram(r2h_int32 rtimeout)
{
	r2h_int32 r = rtimeout;
	sde_datagram_header_t *h = (sde_datagram_header_t*)__datagram;
	while(true)
	{
		socket_recv(__tcp, __datagram, sizeof(sde_datagram_header_t), &r);
		socket_recv(__tcp, __datagram + sizeof(sde_datagram_header_t), h->bytes, &r);
		if(h->func.cmd_flag == 1)
			break;
	}
	if(strncmp((char*)(__datagram + 2), PROTOCOL_IDENTIFIER, sizeof(PROTOCOL_IDENTIFIER)) != 0)
		throw GenericException(SDE_RECV_INVALID_DATAGRAM);
	if(h->func.exception_flag == 1)
		throw GenericException(SDE_RECV_INVALID_DATAGRAM);
	return 0;
}

}
