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
#include "Delay_ms.hh"

/*----------------------------------------------------------------------------
  reset
 *----------------------------------------------------------------------------*/

void reset(bool hard_reset) {
//	ATOMIC_BLOCK(ATOMIC_FORCEON) {
	__disable_irq ();
	SystemCoreClockUpdate();
	while (SysTick_Config(SystemCoreClock / 1000));   /* Setup SysTick Timer for 1 msec interrupts  */
	Motherboard& board = Motherboard::getBoard();
	sdcard::reset();
	steppers::abort();
	command::reset();
	eeprom::init();
	board.reset();
//	__enable_irq ();
	// If we've just come from a hard reset, wait for 2.5 seconds before
	// trying to ping an extruder.  This gives the extruder time to boot
	// before we send it a packet.
	if (hard_reset) {
		Timeout t;
		t.start(1000L*2500L); // wait for 2500 ms
		while (!t.hasElapsed());
	}
	if (!tool::reset())
	{
		// Fail, but let it go; toggling the PSU is dangerous.
	}
	__enable_irq ();
//	}
}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

	Motherboard& board = Motherboard::getBoard();
	steppers::init(Motherboard::getBoard());
	reset(true);
	__enable_irq ();
	while (1) {
		// Toolhead interaction thread.
		tool::runToolSlice();
		// Host interaction thread.
		host::runHostSlice();
		// Command handling thread.
		command::runCommandSlice();
		// Motherboard slice
		board.runMotherboardSlice();
	}
	return 0;
}

