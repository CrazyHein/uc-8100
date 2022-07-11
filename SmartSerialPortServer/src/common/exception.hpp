/*
 * exception.hpp
 *
 *  Created on: Jun 21, 2022
 *      Author: r2h
 */

#ifndef COMMON_EXCEPTION_HPP_
#define COMMON_EXCEPTION_HPP_


#include "data_type.hpp"
#include "error_code_def.hpp"
#include <exception>

namespace common
{

class GenericException : public std::exception
{
public:
	explicit GenericException(r2h_int32 code);
	GenericException(std::exception &e);

	virtual ~GenericException();

	r2h_int32 ErrorCode() const;
	const std::exception& InnerException() const;

private:
	r2h_int32 __error_code;
	std::exception __inner_exception;
};

class SysDriverException : public GenericException
{
public:
	explicit SysDriverException(r2h_int32 code);
	virtual ~SysDriverException();
};

class SysResourceException : public GenericException
{
public:
	explicit SysResourceException(r2h_int32 code);
	virtual ~SysResourceException();
};

}

#endif /* COMMON_EXCEPTION_HPP_ */
