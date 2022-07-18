/*
 * moxa_led.cpp
 *
 *  Created on: Jul 3, 2022
 *      Author: r2h
 */
#include "moxa_led.hpp"

namespace utility
{
static int __MOXA_LED_LIB_INITIALIZED = 0;

void set_programmable_led(r2h_int32 group, r2h_int32 index, LED_STATE_T state)
{
	int res = 0;

	if(__MOXA_LED_LIB_INITIALIZED == 0)
	{
		res = mx_led_init();
		if(res < 0)
			throw SysDriverException(MOXA_API_LED_INIT_EXCEPTION);
		__MOXA_LED_LIB_INITIALIZED = 1;
	}

	res = mx_led_set_brightness(LED_TYPE_PROGRAMMABLE, group, index, (r2h_int32)state);
	if(res < 0)
		throw SysDriverException(MOXA_API_LED_OP_EXCEPTION);
}

void set_signal_led(r2h_int32 group, r2h_int32 index, LED_STATE_T state)
{
	int res = 0;

	if(__MOXA_LED_LIB_INITIALIZED == 0)
	{
		res = mx_led_init();
		if(res < 0)
			throw SysDriverException(MOXA_API_LED_INIT_EXCEPTION);
		__MOXA_LED_LIB_INITIALIZED = 1;
	}

	res = mx_led_set_brightness(LED_TYPE_SIGNAL, group, index, (r2h_int32)state);
	if(res < 0)
		throw SysDriverException(MOXA_API_LED_OP_EXCEPTION);
}

void set_uc8100_green_led(LED_STATE_T state)
{
	int res = 0;

	if(__MOXA_LED_LIB_INITIALIZED == 0)
	{
		res = mx_led_init();
		if(res < 0)
			throw SysDriverException(MOXA_API_LED_INIT_EXCEPTION);
		__MOXA_LED_LIB_INITIALIZED = 1;
	}

	res = mx_led_set_brightness(LED_TYPE_PROGRAMMABLE, 1, 3, (r2h_int32)state);
	if(res < 0)
		throw SysDriverException(MOXA_API_LED_OP_EXCEPTION);
}

void set_uc8100_yellow_led(LED_STATE_T state)
{
	int res = 0;

	if(__MOXA_LED_LIB_INITIALIZED == 0)
	{
		res = mx_led_init();
		if(res < 0)
			throw SysDriverException(MOXA_API_LED_INIT_EXCEPTION);
		__MOXA_LED_LIB_INITIALIZED = 1;
	}

	res = mx_led_set_brightness(LED_TYPE_PROGRAMMABLE, 1, 2, (r2h_int32)state);
	if(res < 0)
		throw SysDriverException(MOXA_API_LED_OP_EXCEPTION);
}

void set_uc8100_red_led(LED_STATE_T state)
{
	int res = 0;

	if(__MOXA_LED_LIB_INITIALIZED == 0)
	{
		res = mx_led_init();
		if(res < 0)
			throw SysDriverException(MOXA_API_LED_INIT_EXCEPTION);
		__MOXA_LED_LIB_INITIALIZED = 1;
	}

	res = mx_led_set_brightness(LED_TYPE_PROGRAMMABLE, 1, 1, (r2h_int32)state);
	if(res < 0)
		throw SysDriverException(MOXA_API_LED_OP_EXCEPTION);
}

void set_led_all_off()
{
	int res = 0;

	if(__MOXA_LED_LIB_INITIALIZED == 0)
	{
		res = mx_led_init();
		if(res < 0)
			throw SysDriverException(MOXA_API_LED_INIT_EXCEPTION);
		__MOXA_LED_LIB_INITIALIZED = 1;
	}

	res = mx_led_set_all_off();
	if(res < 0)
		throw SysDriverException(MOXA_API_LED_OP_EXCEPTION);
}

void set_led_all_on()
{
	int res = 0;

	if(__MOXA_LED_LIB_INITIALIZED == 0)
	{
		res = mx_led_init();
		if(res < 0)
			throw SysDriverException(MOXA_API_LED_INIT_EXCEPTION);
		__MOXA_LED_LIB_INITIALIZED = 1;
	}

	res = mx_led_set_all_on();
	if(res < 0)
		throw SysDriverException(MOXA_API_LED_OP_EXCEPTION);
}


}
