/*
 * moxa_led.hpp
 *
 *  Created on: Jul 3, 2022
 *      Author: r2h
 */

#ifndef UTILITY_MOXA_LED_HPP_
#define UTILITY_MOXA_LED_HPP_

#include <mx_led.h>
#include "../common/data_type.hpp"
#include "../common/error_code_def.hpp"
#include "../common/exception.hpp"

using namespace common;

namespace utility
{
typedef enum LED_STATE {
	LED_STATE_OFF = 0,
	LED_STATE_ON = 1,
	LED_STATE_BLINK = 2
}LED_STATE_T;

void set_programmable_led(r2h_int32 group, r2h_int32 index, LED_STATE_T state);

void set_uc8100_green_led(LED_STATE_T state);

void set_uc8100_yellow_led(LED_STATE_T state);

void set_uc8100_red_led(LED_STATE_T state);

void set_led_all_off();

void set_led_all_on();
}





#endif /* UTILITY_MOXA_LED_HPP_ */
