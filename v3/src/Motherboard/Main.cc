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
#include "Uart32.c"
	#include "lpc17xx_timer.h"
	#include "LPC17xx.h"
	#include "lpc17xx_clkpwr.h"
//test_led(1);
/********************************/

/*----------------------------------------------------------------------------
  reset
 *----------------------------------------------------------------------------*/

void reset(bool hard_reset) {
	__disable_irq ();
	Motherboard& board = Motherboard::getBoard();
	uint8_t menu2[] = "get board\r";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu2, sizeof(menu2), BLOCKING);
	sdcard::reset();
	uint8_t menu3[] = "sdcard reset\r";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu3, sizeof(menu3), BLOCKING);
	steppers::abort();
	uint8_t menu4[] = "stepper abort\r";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu4, sizeof(menu4), BLOCKING);
	command::reset();
	uint8_t menu5[] = "command reset\r";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu5, sizeof(menu5), BLOCKING);
//	eeprom::init();
	board.reset();
	__enable_irq ();
	// If we've just come from a hard reset, wait for 2.5 seconds before
	// trying to ping an extruder.  This gives the extruder time to boot
	// before we send it a packet.
	if (hard_reset) {
		uint8_t menu6[] = "before starting timeout\r";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu6, sizeof(menu6), BLOCKING);
		Timeout t;
		t.start(10L); // wait for 2500 ms
		uint8_t menu7[] = "1\r";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu7, sizeof(menu7), BLOCKING);
		UART_32_DEC((LPC_UART_TypeDef *)LPC_UART2, t.hasLeft());
		uint8_t menu8[] = "2\r";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu8, sizeof(menu8), BLOCKING);
		UART_32_DEC((LPC_UART_TypeDef *)LPC_UART2, t.hasLeft());
		uint8_t menu9[] = "3\r";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu9, sizeof(menu9), BLOCKING);
		UART_32_DEC((LPC_UART_TypeDef *)LPC_UART2, t.hasLeft());
		uint8_t menu10[] = "4\r";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu10, sizeof(menu10), BLOCKING);
		UART_32_DEC((LPC_UART_TypeDef *)LPC_UART2, t.hasLeft());

		uint64_t getPClockkk = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_TIMER1);

		uint64_t clkdlycnttt = (getPClockkk * 100000) / 1000000;


		UART_8((LPC_UART_TypeDef *)LPC_UART2, TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT));
		UART_32_DEC((LPC_UART_TypeDef *)LPC_UART2, (uint32_t)getPClockkk);

		while (!t.hasElapsed());
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
	NVIC_SetPriorityGrouping(1);					// Configure the NVIC Preemption Priority Bits
	//----end of Initialization of LPC----//
	test_u();
//	UART_SendByte ((LPC_UART_TypeDef *)LPC_UART2, 4);
	uint8_t menu1[] = "\rStart\r";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu1, sizeof(menu1), BLOCKING);
	UART_32_DEC((LPC_UART_TypeDef *)LPC_UART2, SystemCoreClock);
	Motherboard& board = Motherboard::getBoard();
	steppers::init(Motherboard::getBoard());
	reset(true);
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

