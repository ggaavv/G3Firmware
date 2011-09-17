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

#ifndef MOTHERBOARD_HH_
#define MOTHERBOARD_HH_

//
// This file describes the Motherboard object, which provides interfaces for
// all facilities provided by the motherboard.  The Motherboard is a singleton;
// call Motherboard::getBoard() to get a reference to the board.
//
// The board should be initialized before use or on reset by calling the init()
// method.
//

#include "UART.hh"
#include "StepperInterface.hh"
#include "Types.hh"
#include "Configuration.hh"
#include "Timeout.hh"

class Motherboard {
private:
	const static int STEPPERS = STEPPER_COUNT;

	StepperInterface stepper[STEPPERS];
	/// Microseconds since board initialization
	volatile micros_t micros;
	/// Private constructor; use the singleton
	Motherboard();

	static Motherboard motherboard;

	Timeout interface_update_timeout;

	// True if we have an interface board attached
	bool hasInterfaceBoard;

public:
	/// Reset the motherboard to its initial state.
	/// This only resets the board, and does not send a reset
	/// to any attached toolheads.
	void reset();

	void runMotherboardSlice();

	/// Count the number of steppers available on this board.
	const int getStepperCount() const { return STEPPERS; }
	/// Get the stepper interface for the nth stepper.
	StepperInterface& getStepperInterface(int n)
	{
		return stepper[n];
	}

	/// Get the number of microseconds that have passed since
	/// the board was initialized.  This value will wrap after
	/// 2**32 microseconds (ca. 70 minutes); callers should compensate for this.
	micros_t getCurrentMicros();

	/// Get the power supply unit interface.
	// PSU& getPSU() { return psu; }

	/// Write an error code to the debug pin.
	void indicateError(int errorCode);
	/// Get the current error being displayed.
	uint8_t getCurrentError();

	/// Get the motherboard instance.
	static Motherboard& getBoard() { return motherboard; }

	/// Perform the timer interrupt routine.
	void doInterrupt();
};

#endif // MOTHERBOARD_HH_
