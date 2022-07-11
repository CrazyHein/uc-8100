/*
 * generic_shared_memory.hpp
 *
 *  Created on: Jun 25, 2022
 *      Author: r2h
 */

#ifndef ETH_PORT_SERVER_MEMORY_GENERIC_SHARED_MEMORY_HPP_
#define ETH_PORT_SERVER_MEMORY_GENERIC_SHARED_MEMORY_HPP_

#include <pthread.h>
#include "../../common/data_type.hpp"

using namespace common;

namespace ethernet_port_server
{
class GenericSharedMemory
{
public:
	GenericSharedMemory(r2h_uint16 size, pthread_mutex_t *pMutex);
	GenericSharedMemory(const GenericSharedMemory&) = delete;
	GenericSharedMemory(GenericSharedMemory&&) = delete;

	virtual ~GenericSharedMemory();

	void Read(r2h_uint16 start, r2h_uint16 size, r2h_byte* pData) const;
	void Write(r2h_uint16 start, r2h_uint16 size, const r2h_byte* pData);
	r2h_uint16 Size() const;
private:
	r2h_uint16 __limit;
	r2h_byte* __data_ptr;
	pthread_mutex_t __mutex;
	pthread_mutex_t* __mutex_ptr;
	bool __internal_mutex;

};
}





#endif /* ETH_PORT_SERVER_MEMORY_GENERIC_SHARED_MEMORY_HPP_ */
