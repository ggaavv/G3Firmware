/**************************************************************************//**
 * @file     main.c
 * @brief    CMSIS Cortex-M3 Blinky example
 *           Blink a LED using CM3 SysTick
 * @version  V1.03
 * @date     24. September 2009
 *
 * @note
 * Copyright (C) 2009 ARM Limited. All rights reserved.
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Cortex-M 
 * processor based microcontrollers.  This file can be freely distributed 
 * within development tools that are supporting such ARM based processors. 
 *
 * @par
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/

extern "C" {
	#include "LPC17xx.h"
}
#include "DebugPacketProcessor.hh"
#include "Host.hh"
#include "Tool.hh"
#include "Command.hh"
//#include <avr/interrupt.h>    //NEED remove
//#include <util/atomic.h>		//NEED remove
#include "Timeout.hh"
#include "Steppers.hh"
#include "Motherboard.hh"
#include "SDCard.hh"
#include "EepromMap.hh"
#include "Main.hh"
#include "Delay.hh"
#include "test.hh"  // testing
/********************************/
#include "test_led.hh"  // testing
#include "test_u.hh"
//test_led(1);
/********************************/

/*----------------------------------------------------------------------------
  reset
 *----------------------------------------------------------------------------*/

void reset(bool hard_reset) {
	__disable_irq ();
	Motherboard& board = Motherboard::getBoard();
	test_led(100);
	sdcard::reset();
	steppers::abort();
	command::reset();
//	eeprom::init();
	board.reset();
	test_led(10);
	__enable_irq ();
	// If we've just come from a hard reset, wait for 2.5 seconds before
	// trying to ping an extruder.  This gives the extruder time to boot
	// before we send it a packet.
	if (hard_reset) {
		test_led3(4);
		Timeout t;
		t.start(10L); // wait for 2500 ms
		test_led3(5);
		while (!t.hasElapsed());
		test_led3(6);
	}
	if (!tool::reset())
	{
		// Fail, but let it go; toggling the PSU is dangerous.
	}
}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
	/* NOTE: you will need to call SystemCoreClockUpdate() as the very
	first line in your main function. This will update the various
	registers and constants to allow accurate timing. */
	SystemCoreClockUpdate();
	SystemInit();									// Initialize clocks
	//----Initialization of LPC----//
	test_led3(1);
	NVIC_SetPriorityGrouping(1);					// Configure the NVIC Preemption Priority Bits
	//----end of Initialization of LPC----//
	test_u();
	test_led3(1);
	UART_SendByte (LPC_UART2, 0x7);
	UART_SendByte (LPC_UART2, 0x8);
	UART_SendByte (LPC_UART2, 0x9);
	test_led(2);
	Motherboard& board = Motherboard::getBoard();
	steppers::init(Motherboard::getBoard());
	reset(true);
	while (1) {
		test_led2(10);
		// Toolhead interaction thread.
		tool::runToolSlice();
		test_led2(20);
		// Host interaction thread.
		host::runHostSlice();
		test_led2(30);
		// Command handling thread.
		command::runCommandSlice();
		test_led2(40);
		// Motherboard slice
		board.runMotherboardSlice();
		test_led(5);
	}
	return 0;
}

