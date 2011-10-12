
#ifndef _TEST_LED_HH_
#define _TEST_LED_HH_

extern "C" {
	#include "lpc17xx_uart.h"
	#include "LPC17xx.h"
}


extern void test_led(uint8_t number);
extern void test_led2(uint8_t number);
extern void test_led3(uint8_t number);

#endif /* _TEST_LED_HH_ */
