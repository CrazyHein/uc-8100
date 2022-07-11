/*
 * exception.cpp
 *
 *  Created on: Jun 21, 2022
 *      Author: r2h
 */
#include "exception.hpp"
namespace common
{
GenericException::GenericException(r2h_int32 code):__error_code(code), __inner_exception()
{

}

GenericException::GenericException(std::exception &e):__error_code(CPLUS_STD_EXCEPTION), __inner_exception(e)
{

}

GenericException::~GenericException()
{

}

r2h_int32 GenericException::ErrorCode() const
{
	return __error_code;
}

const std::exception& GenericException::InnerException() const
{
	return __inner_exception;
}

SysDriverException::SysDriverException(r2h_int32 code) : GenericException(code)
{

}

SysDriverException::~SysDriverException()
{

}

SysResourceException::SysResourceException(r2h_int32 code) : GenericException(code)
{

}

SysResourceException::~SysResourceException()
{

}
}
