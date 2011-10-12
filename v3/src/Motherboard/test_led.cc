/*Copyright (C) 2011 by Sagar G V

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "test_led.hh"
#include "Delay.hh"

/***********LED Blinky Example**********************************************************************/
void test_led(uint8_t number)
{
//	LPC_GPIO1->FIODIR |= 1 << 29; // P1.29 connected to LED
	LPC_GPIO1->FIODIR |= 1 << 27; // 1.27 Y_MIN
	uint8_t number1;
	number1 = number;
	uint8_t i;
	for ( i=0 ; i < number1;i++){
//		LPC_GPIO1->FIOPIN ^= 1 << 29; // Toggle P1.29
		LPC_GPIO1->FIOPIN = 1 << 27; // Toggle P1.27
		_delay_ms(10); // wait for approx 400 ms
		LPC_GPIO1->FIOPIN = 0 << 27; // Toggle P1.27
		_delay_ms(10); // wait for approx 400 ms
	}
	_delay_ms(500);
}

void test_led2(uint8_t number)
{
//	LPC_GPIO1->FIODIR |= 1 << 29; // P1.29 connected to LED
	LPC_GPIO1->FIODIR |= 1 << 23; // 1.23 Y_MAX
	uint8_t number1;
	number1 = number;
	uint8_t i;
	for ( i=0 ; i < number1;i++){
//		LPC_GPIO1->FIOPIN ^= 1 << 29; // Toggle P1.29
		LPC_GPIO1->FIOPIN = 1 << 23; // Toggle P1.23
		_delay_ms(10); // wait for approx 400 ms
		LPC_GPIO1->FIOPIN = 0 << 23; // Toggle P1.23
		_delay_ms(10); // wait for approx 400 ms
	}
	_delay_ms(500);
}

void test_led3(uint8_t number)
{
//	LPC_GPIO1->FIODIR |= 1 << 29; // P1.29 connected to LED
	LPC_GPIO1->FIODIR |= 1 << 22; // 1.27 Z_MIN
	uint8_t number1;
	number1 = number;
	uint8_t i;
	for ( i=0 ; i < number1;i++){
//		LPC_GPIO1->FIOPIN ^= 1 << 29; // Toggle P1.29
		LPC_GPIO1->FIOPIN = 1 << 22; // Toggle P1.27
		_delay_ms(100); // wait for approx 400 ms
		LPC_GPIO1->FIOPIN = 0 << 22; // Toggle P1.27
		_delay_ms(100); // wait for approx 400 ms
	}
	_delay_ms(500);
}
