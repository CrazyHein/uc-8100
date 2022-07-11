/*
 * generic_shared_memory.cpp
 *
 *  Created on: Jun 26, 2022
 *      Author: r2h
 */
#include "generic_shared_memory.hpp"
#include "../../common/exception.hpp"
#include <string.h>

namespace ethernet_port_server
{

GenericSharedMemory::GenericSharedMemory(r2h_uint16 size, pthread_mutex_t *pMutex) : __limit(size), __data_ptr(nullptr), __mutex_ptr(pMutex), __internal_mutex(false)
{
	__mutex = {0};
	pthread_mutexattr_t mattr = {0};

	try
	{
		__data_ptr = new r2h_byte[size];
		memset(__data_ptr, 0, size);

		if(pMutex == nullptr)
		{
			pthread_mutexattr_init(&mattr);
			if(pthread_mutex_init(&__mutex, &mattr) != 0)
				throw SysResourceException(SYS_OUT_OF_RESOURCE);
			pthread_mutexattr_destroy(&mattr);
			__mutex_ptr = &__mutex;
			__internal_mutex = true;
		}
	}
	catch(SysResourceException &e)
	{
		pthread_mutexattr_destroy(&mattr);
		throw;
	}
	catch(std::exception &e)
	{
		throw SysResourceException(SYS_OUT_OF_MEMORY);
	}
}

GenericSharedMemory::~GenericSharedMemory()
{
	if(__internal_mutex)
		pthread_mutex_destroy(&__mutex);
	if(__data_ptr != nullptr)
		delete[] __data_ptr;
}

void GenericSharedMemory::Read(r2h_uint16 start, r2h_uint16 size, r2h_byte* pData) const
{
	if(start + size > __limit || start >= __limit)
		throw GenericException(MEM_ACCESS_OUT_OF_RANGE);
	pthread_mutex_lock(__mutex_ptr);
	memcpy(pData, __data_ptr + start, size);
	pthread_mutex_unlock(__mutex_ptr);
}

void GenericSharedMemory::Write(r2h_uint16 start, r2h_uint16 size, const r2h_byte* pData)
{
	if(start + size > __limit || start >= __limit)
		throw GenericException(MEM_ACCESS_OUT_OF_RANGE);
	pthread_mutex_lock(__mutex_ptr);
	memcpy(__data_ptr + start, pData, size);
	pthread_mutex_unlock(__mutex_ptr);
}

r2h_uint16 GenericSharedMemory::Size() const
{
	return __limit;
}

}
