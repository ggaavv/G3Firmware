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

#include <stdint.h>
extern "C" {
	#include "lpc17xx_timer.h"
	#include "LPC17xx.h"
	#include "lpc17xx_nvic.h"
}
#include "Motherboard.hh"
#include "Configuration.hh"
#include "Steppers.hh"
#include "Command.hh"
#include "Interface.hh"
#include "Tool.hh"
#include "Commands.hh"

/********************************/
#include "test.hh"  // testing
#include "test_led.hh"  // testing
#include "test_u.hh"
#include "Uart32.c"
//#include "Delay.hh"
//	#include "lpc17xx_nvic.h"
//	#include "lpc17xx_timer.h"
//	#include "LPC17xx.h"
//test_led(1);
/********************************/

/// Number of times to blink the debug LED on each cycle
volatile uint8_t blink_count = 0;

/// The current state of the debug LED
enum debugled{
	BLINK_NONE,
	BLINK_ON,
	BLINK_OFF,
	BLINK_PAUSE
} blink_state = BLINK_NONE;

/// Write an error code to the debug pin.
void Motherboard::indicateError(int error_code) {
	if (error_code == 0) {
		blink_state = BLINK_NONE;
		DEBUG_PIN.setValue(false);
	}
	else if (blink_count != error_code) {
		blink_state = BLINK_OFF;
	}
	blink_count = error_code;
}

/// Get the current error code.
uint8_t Motherboard::getCurrentError() {
	return blink_count;
}

/// Timer2 overflow cycles that the LED remains on while blinking
#define OVFS_ON 18
/// Timer2 overflow cycles that the LED remains off while blinking
#define OVFS_OFF 18
/// Timer2 overflow cycles between flash cycles
#define OVFS_PAUSE 80

/// Number of overflows remaining on the current blink cycle
int blink_ovfs_remaining = 0;
/// Number of blinks performed in the current cycle
int blinked_so_far = 0;


/// Timer one comparator match interrupt
extern "C" void TIMER0_IRQHandler (void){
//	uint8_t menu110[] = "\rQ0";
//	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu110, sizeof(menu110), BLOCKING);
	if((LPC_TIM0->IR & 0x01) == 0x01) {// if MR0 interrupt
	Motherboard::getBoard().doInterrupt();
	}
	TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT);
}

/// Timer 2 overflow interrupt
extern "C" void TIMER1_IRQHandler (void){
//	uint8_t menu10[] = "\rQ1";
//	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu10, sizeof(menu10), BLOCKING);
	if((LPC_TIM1->IR & 0x01) == 0x01) {// if MR0 interrupt
		if (blink_ovfs_remaining > 0) {
			blink_ovfs_remaining--;
		} else {
			if (blink_state == BLINK_ON) {
				blinked_so_far++;
				blink_state = BLINK_OFF;
				blink_ovfs_remaining = OVFS_OFF;
				DEBUG_PIN.setValue(false);
			} else if (blink_state == BLINK_OFF) {
				if (blinked_so_far >= blink_count) {
					blink_state = BLINK_PAUSE;
					blink_ovfs_remaining = OVFS_PAUSE;
				} else {
					blink_state = BLINK_ON;
					blink_ovfs_remaining = OVFS_ON;
					DEBUG_PIN.setValue(true);
				}
			} else if (blink_state == BLINK_PAUSE) {
				blinked_so_far = 0;
				blink_state = BLINK_ON;
				blink_ovfs_remaining = OVFS_ON;
				DEBUG_PIN.setValue(true);
			}
		}
	}
	TIM_ClearIntPending(LPC_TIM1,TIM_MR1_INT);
}

/// Instantiate static motherboard instance
Motherboard Motherboard::motherboard;

/// Create motherboard object
Motherboard::Motherboard()
{
	/// Set up the stepper pins on board creation
#if STEPPER_COUNT > 0
	stepper[0] = StepperInterface(X_DIR_PIN,X_STEP_PIN,X_ENABLE_PIN,X_MAX_PIN,X_MIN_PIN);
#endif
#if STEPPER_COUNT > 1
	stepper[1] = StepperInterface(Y_DIR_PIN,Y_STEP_PIN,Y_ENABLE_PIN,Y_MAX_PIN,Y_MIN_PIN);
#endif
#if STEPPER_COUNT > 2
	stepper[2] = StepperInterface(Z_DIR_PIN,Z_STEP_PIN,Z_ENABLE_PIN,Z_MAX_PIN,Z_MIN_PIN);
#endif
#if STEPPER_COUNT > 3
	stepper[3] = StepperInterface(A_DIR_PIN,A_STEP_PIN,A_ENABLE_PIN,Pin(),Pin());
#endif
#if STEPPER_COUNT > 4
	stepper[4] = StepperInterface(B_DIR_PIN,B_STEP_PIN,B_ENABLE_PIN,Pin(),Pin());
#endif
}

/// Reset the motherboard to its initial state.
/// This only resets the board, and does not send a reset
/// to any attached toolheads.
void Motherboard::reset() {
	micros = 0;	/// Microseconds since board initialization set to 0
	// Configure the debug pin.
	DEBUG_PIN.setDirection(true);
	indicateError(0); // turn off blinker
	// Init steppers
	// NB: for now, we are turning on Z hold for these boards!
	steppers::setHoldZ(true);

	uint8_t menu1161[] = "\rsetHoldZ true";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu1161, sizeof(menu1161), BLOCKING);

	for (int i = 0; i < STEPPER_COUNT; i++) {
		stepper[i].init(5);
	}
	// Initialize the host and slave UARTs

	uint8_t menu161[] = "\rb4 uarts up";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu161, sizeof(menu161), BLOCKING);

	UART::getHostUART().enable(true);
	UART::getHostUART().in.reset();

	uint8_t menu261[] = "\rbetween uarts";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu261, sizeof(menu261), BLOCKING);

	UART::getSlaveUART().enable(true);
	UART::getSlaveUART().in.reset();

	uint8_t menu191[] = "\rafter uarts up";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu191, sizeof(menu191), BLOCKING);

	// Reset and configure timer 1, the microsecond and stepper
	// interrupt timer.
	TIM_TIMERCFG_Type TMR0_Cfg;
	TIM_MATCHCFG_Type TMR0_Match;
	// On reset, Timer0/1 are enabled (PCTIM0/1 = 1), and Timer2/3 are disabled (PCTIM2/3 = 0).
	// Initialize timer 1, prescale count time of 100uS
	TMR0_Cfg.PrescaleOption = TIM_PRESCALE_USVAL;
	TMR0_Cfg.PrescaleValue = 1; // reset to 1 - 1uS
	// Use channel 1, MR1
	TMR0_Match.MatchChannel = 0;
	// Enable interrupt when MR0 matches the value in TC register
	TMR0_Match.IntOnMatch = TRUE;
	// Enable reset on MR0: TIMER will reset if MR0 matches it
	TMR0_Match.ResetOnMatch = TRUE;
	// Don't stop on MR0 if MR0 matches it
	TMR0_Match.StopOnMatch = FALSE;
	// Do nothing for external output pin if match (see cmsis help, there are another options)
	TMR0_Match.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	// Set Match value, count value of INTERVAL_IN_MICROSECONDS (64 * 1uS = 64us )
	TMR0_Match.MatchValue = INTERVAL_IN_MICROSECONDS;
	// Set configuration for Tim_config and Tim_MatchConfig
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &TMR0_Cfg);
	TIM_ConfigMatch(LPC_TIM0, &TMR0_Match);
	// 0 top priority 32 lowest
	NVIC_SetPriority(TIMER0_IRQn, 0);
	NVIC_EnableIRQ(TIMER0_IRQn);
	TIM_Cmd(LPC_TIM0,ENABLE);

	// Reset and configure timer 2, the debug LED flasher timer.
	TIM_TIMERCFG_Type TMR1_Cfg;
	TIM_MATCHCFG_Type TMR1_Match;
	/* On reset, Timer0/1 are enabled (PCTIM0/1 = 1), and Timer2/3 are disabled (PCTIM2/3 = 0).*/
	/* Initialize timer 2, prescale count time of 100uS */
	TMR1_Cfg.PrescaleOption = TIM_PRESCALE_USVAL;
	TMR1_Cfg.PrescaleValue = 10000;  //reset to 10000
	/* Use channel 1, MR1 */
	TMR1_Match.MatchChannel = 1;
	/* Enable interrupt when MR0 matches the value in TC register */
	TMR1_Match.IntOnMatch = ENABLE;
	/* Enable reset on MR0: TIMER will reset if MR0 matches it */
	TMR1_Match.ResetOnMatch = TRUE;
	/* Don't stop on MR0 if MR0 matches it*/
	TMR1_Match.StopOnMatch = FALSE;
	/* Do nothing for external output pin if match (see cmsis help, there are another options) */
	TMR1_Match.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	/* Set Match value, count value of 100 (10 * 10000uS = 100000us = 1s --> 10 Hz) */
	TMR1_Match.MatchValue = 100;
	/* Set configuration for Tim_config and Tim_MatchConfig */
	TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &TMR1_Cfg);
	TIM_ConfigMatch(LPC_TIM1, &TMR1_Match);
	// 0 top priority 32 lowest
	NVIC_SetPriority(TIMER1_IRQn, 17);
	NVIC_EnableIRQ(TIMER1_IRQn);
	TIM_Cmd(LPC_TIM1,ENABLE);

	uint8_t menu171[] = "\rtimers configured";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu171, sizeof(menu171), BLOCKING);

	// Check if the interface board is attached
	hasInterfaceBoard = interfaceboard::isConnected();
//	if (hasInterfaceBoard) {
	if (0) {
		// Make sure our interface board is initialized
		interfaceboard::init();
		interface_update_timeout.start(interfaceboard::getUpdateRate());
	}
    // Blindly try to reset the toolhead with index 0.
//	resetToolhead();
}

/// Get the number of microseconds that have passed since
/// the board was booted.
micros_t Motherboard::getCurrentMicros() {
	micros_t micros_snapshot;
//	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		micros_snapshot = micros;
//	}
	return micros_snapshot;
}


/// Run the motherboard interrupt
void Motherboard::doInterrupt() {
	if (hasInterfaceBoard) {
		interfaceboard::doInterrupt();
	}
	micros += INTERVAL_IN_MICROSECONDS;
	// Do not move steppers if the board is in a paused state
	if (command::isPaused()) return;
	steppers::doInterrupt();
}

void Motherboard::runMotherboardSlice() {
	if (hasInterfaceBoard) {
		if (interface_update_timeout.hasElapsed()) {
			interfaceboard::doUpdate();
			interface_update_timeout.start(interfaceboard::getUpdateRate());
		}
	}
}


