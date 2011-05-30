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

#include "LPC17xx.h"
#include "DebugPacketProcessor.hh"
#include "Host.hh"
#include "Tool.hh"
#include "Command.hh"
#include <avr/interrupt.h>    //NEED remove
#include <util/atomic.h>		//NEED remove
#include "Timeout.hh"
#include "Steppers.hh"
#include "Motherboard.hh"
#include "SDCard.hh"
#include "EepromMap.hh"


volatile uint32_t msTicks;                            /* counts 1ms timeTicks */
/*----------------------------------------------------------------------------
  SysTick_Handler
 *----------------------------------------------------------------------------*/
void SysTick_Handler(void) {
  msTicks++;                        /* increment counter necessary in Delay() */
}

/*------------------------------------------------------------------------------
  delays number of tick Systicks (happens every 1 ms)
 *------------------------------------------------------------------------------*/
__INLINE static void Delay (uint32_t dlyTicks) {
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks);
}

/*------------------------------------------------------------------------------
  configer LED pins
 *------------------------------------------------------------------------------*/
__INLINE static void LED_Config(void) {

  LPC_GPIO1->FIODIR = 0xFFFFFFFF;               /* LEDs PORT1 are Output */
}

/*------------------------------------------------------------------------------
  Switch on LEDs
 *------------------------------------------------------------------------------*/
__INLINE static void LED_On (uint32_t led) {

  LPC_GPIO1->FIOPIN |=  (led);                  /* Turn On  LED */
}


/*------------------------------------------------------------------------------
  Switch off LEDs
 *------------------------------------------------------------------------------*/
__INLINE static void LED_Off (uint32_t led) {

  LPC_GPIO1->FIOPIN &= ~(led);                  /* Turn Off LED */
}

/*----------------------------------------------------------------------------
  reset
 *----------------------------------------------------------------------------*/

void reset(bool hard_reset) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		Motherboard& board = Motherboard::getBoard();
		sdcard::reset();
		steppers::abort();
		command::reset();
		eeprom::init();
		board.reset();
		sei();
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
	}
}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {

	Motherboard& board = Motherboard::getBoard();
	steppers::init(Motherboard::getBoard());
	reset(true);
	sei();		// NEED - enables global interupsts
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






  if (SysTick_Config(SystemCoreClock / 1000)) { /* Setup SysTick Timer for 1 msec interrupts  */
    while (1);                                  /* Capture error */
  }
  
  LED_Config();                             
 
  while(1) {
    LED_On ((1<<18));                           /* Turn on the LED. */
    Delay (100);                                /* delay  100 Msec */
    LED_Off ((1<<18));                          /* Turn off the LED. */
    Delay (100);                                /* delay  100 Msec */
  }
  
}

