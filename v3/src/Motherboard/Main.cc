/*
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

extern "C" {
	#include "LPC17xx.h"
	#include "vcomdemo.h"
}
#include "DebugPacketProcessor.hh"
#include "Host.hh"
#include "Tool.hh"
#include "Command.hh"
#include "Atomic.hh"
#include "Timeout.hh"
#include "Steppers.hh"
#include "Motherboard.hh"
#include "SDCard.hh"
#include "EepromMap.hh"
#include "Main.hh"
#include "Delay.hh"
/********************************/
#include "UART.hh"
#include "test.hh"  // testing
#include "test_led.hh"  // testing
#include "test_u.hh"
//#include "Delay.hh"
//#include "lpc17xx_nvic.h"
//#include "lpc17xx_timer.h"
//#include "LPC17xx.h"
//test_led(1);
#include "Uart32.h"
/********************************/

/*----------------------------------------------------------------------------
  reset
 *----------------------------------------------------------------------------*/

void reset(bool hard_reset) {
//	__disable_irq ();
	Atomic(BEGIN_INT);
	Motherboard& board = Motherboard::getBoard();
	uint8_t menum2[] = "get board\n";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menum2, sizeof(menum2), BLOCKING);
	sdcard::reset();
	uint8_t menum3[] = "sdcard reset\n";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menum3, sizeof(menum3), BLOCKING);
	steppers::abort();
	uint8_t menum4[] = "stepper abort\n";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menum4, sizeof(menum4), BLOCKING);
	command::reset();
	uint8_t menum5[] = "command reset\n";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menum5, sizeof(menum5), BLOCKING);
	eeprom::init();
//	__disable_irq ();
	board.reset();
//	__enable_irq ();
	Atomic(ENABLE_INT);
	// If we've just come from a hard reset, wait for 2.5 seconds before
	// trying to ping an extruder.  This gives the extruder time to boot
	// before we send it a packet.
	if (hard_reset) {
//		uint8_t menu6[] = "before starting timeout\n";
//		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu6, sizeof(menu6), BLOCKING);
		Timeout t;
		t.start(1000L); // wait for 2500 ms
/*		uint8_t menu7[] = "1\n";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu7, sizeof(menu7), BLOCKING);
//		UART_32_DEC((LPC_UART_TypeDef *)LPC_UART2, t.hasLeft());
		uint8_t menu8[] = "2\n";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu8, sizeof(menu8), BLOCKING);
//		UART_32_DEC((LPC_UART_TypeDef *)LPC_UART2, t.hasLeft());
//		UART_32_DEC((LPC_UART_TypeDef *)LPC_UART2, t.hasLeft());
//		uint8_t menu10[] = "\n";
//		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu10, sizeof(menu10), BLOCKING);
//		UART_32_DEC((LPC_UART_TypeDef *)LPC_UART2, t.hasLeft());

//		uint64_t getPClockkk = CLKPWR_GetPCLK (CLKPWR_PCLKSEL_TIMER1);

//		uint64_t clkdlycnttt = (getPClockkk * 100000) / 1000000;


//		UART_8((LPC_UART_TypeDef *)LPC_UART2, TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT));
//		UART_32_DEC((LPC_UART_TypeDef *)LPC_UART2, (uint32_t)clkdlycnttt);
//		test_led3(1);
*/
		uint8_t menum1866[] = "t.hasElapsed\n";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menum1866, sizeof(menum1866), BLOCKING);
		while (!t.hasElapsed());
		uint8_t menum866[] = "b4 test\n";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menum866, sizeof(menum866), BLOCKING);
		tool::test(); // Run test
		uint8_t menum66[] = "after test\n";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menum66, sizeof(menum66), BLOCKING);
	}
	if (!tool::reset())
	{
		// Fail, but let it go; toggling the PSU is dangerous.
	}
	uint8_t menum76[] = "tool reset done\n";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menum76, sizeof(menum76), BLOCKING);
}

/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
	//----Initialization of LPC----//
	/* NOTE: you will need to call SystemCoreClockUpdate() as the very
	first line in your main function. This will update the various
	registers and constants to allow accurate timing. */
	SystemCoreClockUpdate();
	SystemInit();									// Initialize clocks
	NVIC_SetPriorityGrouping(0);					// Configure the NVIC Preemption Priority Bits
	DEBUG_PIN.setDirection(true);	//creates Port Class
	DEBUG_PIN.setValue(false);
	//----end of Initialization of LPC----//
	test_u();
	uint8_t menum1[] = "\nS\n";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menum1, sizeof(menum1), BLOCKING);
//	UART_32_DEC((LPC_UART_TypeDef *)LPC_UART2, SCB->VTOR);
	_delay_ms(500);
	DEBUG_PIN.setValue(true);
	Motherboard& board = Motherboard::getBoard();
	steppers::init(Motherboard::getBoard());
	reset(true);
	VCOM_Start();
	Atomic(ENABLE_INT);
	while (1) {
//		command::push(0x77);
		// Toolhead interaction thread.
		tool::runToolSlice();
		// Host interaction thread.
//		uint8_t menu111[] = "tool slice run\n";
//		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu111, sizeof(menu111), BLOCKING);
		host::runHostSlice();
		// Command handling thread.
//		uint8_t menu112[] = "host slice run\n";
//		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu112, sizeof(menu112), BLOCKING);
		command::runCommandSlice();
		// Motherboard slice
//		uint8_t menu113[] = "c\n";
//		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu113, sizeof(menu113), BLOCKING);
		board.runMotherboardSlice();
		// USB coms slice
//		VCOM_Usb2Serial();
//		test_led3(1);
	}
	return 0;
}
