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

#include "Command.hh"
#include "Steppers.hh"
#include "Commands.hh"
#include "Tool.hh"
#include "Configuration.hh"
#include "Timeout.hh"
#include "CircularBuffer.hh"
//#include <util/atomic.h>
#include "Atomic.hh"
//#include <avr/eeprom.h>
#include "EepromMap.hh"
#include "SDCard.hh"
/********************************/
//#include "test.hh"  // testing
//#include "test_led.hh"  // testing
//#include "test_u.hh"
#include "Delay.hh"
//	#include "lpc17xx_timer.h"
//	#include "LPC17xx.h"
//	#include "lpc17xx_clkpwr.h"
//test_led(1);
#include "Uart32.h"
/********************************/

namespace command {

//#define COMMAND_BUFFER_SIZE 512	//512 1024 2048 4096 8192  (512 default)  //TODO hack to get circular buffer working
uint8_t buffer_data[COMMAND_BUFFER_SIZE];
CircularBuffer command_buffer(COMMAND_BUFFER_SIZE, buffer_data);

bool outstanding_tool_command = false;

bool paused = false;

uint16_t getRemainingCapacity() {
	uint16_t sz;
//	ATOMIC_BLOCK(ATOMIC_FORCEON) {
//	Only 16bit number so Atomic not needed
		sz = command_buffer.getRemainingCapacity();
//	}
		return sz;
	}

	void pause(bool pause) {
		paused = pause;
	}

	bool isPaused() {
		return paused;
	}

	bool isEmpty() {
		return command_buffer.isEmpty();
	}

	void push(uint8_t byte) {
		command_buffer.push(byte);
	}

	uint8_t pop8() {
		return command_buffer.pop();
	}

	int16_t pop16() {
		union {
			// AVR is little-endian
			int16_t a;
			struct {
				uint8_t data[2];
			} b;
		} shared;
		shared.b.data[0] = command_buffer.pop();
		shared.b.data[1] = command_buffer.pop();
		return shared.a;
	}

	int32_t pop32() {
		union {
			// AVR is little-endian
			int32_t a;
			struct {
				uint8_t data[4];
			} b;
		} shared;
		shared.b.data[0] = command_buffer.pop();
		shared.b.data[1] = command_buffer.pop();
		shared.b.data[2] = command_buffer.pop();
		shared.b.data[3] = command_buffer.pop();
		return shared.a;
	}

	enum {
		READY,
		MOVING,
		DELAY,
		HOMING,
		WAIT_ON_TOOL,
		WAIT_ON_PLATFORM
	} mode = READY;

	Timeout delay_timeout;
	Timeout homing_timeout;
	Timeout tool_wait_timeout;

	void reset() {
		command_buffer.reset();
		mode = READY;
		outstanding_tool_command = false;
		paused = false;
	}

	// A fast slice for processing commands and refilling the stepper queue, etc.
	void runCommandSlice() {
//		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, paused);
//		_delay_ms(10);
//		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x77);
//		_delay_ms(10);
		if (sdcard::isPlaying()) {
			while (command_buffer.getRemainingCapacity() > 0 && sdcard::playbackHasNext()) {
				command_buffer.push(sdcard::playbackNext());
			}
		}
		if (paused) { return; }
		if (mode == HOMING) {
			if (!steppers::isRunning()) {
				mode = READY;
			} else if (homing_timeout.hasElapsed()) {
				steppers::abort();
				mode = READY;
			}
		}
		if (mode == MOVING) {
			if (!steppers::isRunning()) { mode = READY; }
		}
		if (mode == DELAY) {
			// check timers
			if (delay_timeout.hasElapsed()) {
				mode = READY;
			}
		}
		if (mode == WAIT_ON_TOOL) {
			if (tool_wait_timeout.hasElapsed()) {
				mode = READY;
			} else if (tool::getLock()) {
				OutPacket& out = tool::getOutPacket();
				InPacket& in = tool::getInPacket();
				out.reset();
				out.append8(tool::getCurrentToolheadIndex());
				out.append8(SLAVE_CMD_GET_TOOL_STATUS);
				tool::startTransaction();
				// WHILE: bounded by timeout in runToolSlice
				while (!tool::isTransactionDone()) {
					tool::runToolSlice();
				}
				if (!in.hasError()) {
					if (in.read8(1) & 0x01) {
						mode = READY;
					}
				}
				tool::releaseLock();
			}
		}
		if (mode == WAIT_ON_PLATFORM) {
			// FIXME: Duplicates most code from WAIT_ON_TOOL
			if (tool_wait_timeout.hasElapsed()) {
				mode = READY;
			} else if (tool::getLock()) {
				OutPacket& out = tool::getOutPacket();
				InPacket& in = tool::getInPacket();
				out.reset();
				out.append8(tool::getCurrentToolheadIndex());
				out.append8(SLAVE_CMD_IS_PLATFORM_READY);
				tool::startTransaction();
				// WHILE: bounded by timeout in runToolSlice
				while (!tool::isTransactionDone()) {
					tool::runToolSlice();
				}
				if (!in.hasError()) {
					if (in.read8(1) != 0) {
						mode = READY;
					}
				}
				tool::releaseLock();
			}
		}
//		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, mode);
		if (mode == READY) {
//			UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, lengthbuff);
//			_delay_ms(10);
//			UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x44);
//			_delay_ms(10);
			// process next command on the queue.

			if (command_buffer.getLength() > 0) {
				uint8_t command = command_buffer[0];
//				UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, command);
//				_delay_ms(10);
//				UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x88);
//				_delay_ms(10);
				if (command == HOST_CMD_QUEUE_POINT_ABS) {
					uint8_t command = command_buffer[0];
//					UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, command);
//					_delay_ms(50);
//					UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x99);
//					_delay_ms(50);
					// check for completion
					if (command_buffer.getLength() >= 17) {
						command_buffer.pop(); // remove the command code
						mode = MOVING;
						int32_t x = pop32();
						int32_t y = pop32();
						int32_t z = pop32();
						int32_t dda = pop32();
						steppers::setTarget(Point(x,y,z),dda);
					}
				} else if (command == HOST_CMD_QUEUE_POINT_EXT) {
					// check for completion
//					UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x55555);
					if (command_buffer.getLength() >= 25) {
						command_buffer.pop(); // remove the command code
						mode = MOVING;
						int32_t x = pop32();
						int32_t y = pop32();
						int32_t z = pop32();
						int32_t a = pop32();
						int32_t b = pop32();
						int32_t dda = pop32();
						steppers::setTarget(Point(x,y,z,a,b),dda);
					}
				} else if (command == HOST_CMD_QUEUE_POINT_NEW) {
					// check for completion
//					UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x789);
					if (command_buffer.getLength() >= 26) {
						command_buffer.pop(); // remove the command code
						mode = MOVING;
						int32_t x = pop32();
						int32_t y = pop32();
						int32_t z = pop32();
						int32_t a = pop32();
						int32_t b = pop32();
						int32_t us = pop32();
						uint8_t relative = pop8();
						steppers::setTargetNew(Point(x,y,z,a,b),us,relative);
					}
				} else if (command == HOST_CMD_CHANGE_TOOL) {
					if (command_buffer.getLength() >= 2) {
						command_buffer.pop(); // remove the command code
	                                        tool::setCurrentToolheadIndex(command_buffer.pop());
					}
				} else if (command == HOST_CMD_ENABLE_AXES) {
					if (command_buffer.getLength() >= 2) {
						command_buffer.pop(); // remove the command code
						uint8_t axes = command_buffer.pop();
						bool enable = (axes & 0x80) != 0;
						for (int i = 0; i < STEPPER_COUNT; i++) {
							if ((axes & (1 << (i))) != 0) {
								steppers::enableAxis(i, enable);
							}
						}
					}
				} else if (command == HOST_CMD_SET_POSITION) {
					// check for completion
					if (command_buffer.getLength() >= 13) {
						command_buffer.pop(); // remove the command code
						int32_t x = pop32();
						int32_t y = pop32();
						int32_t z = pop32();
						steppers::definePosition(Point(x,y,z));
					}
				} else if (command == HOST_CMD_SET_POSITION_EXT) {
					// check for completion
//					UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x33333);
					if (command_buffer.getLength() >= 21) {
						command_buffer.pop(); // remove the command code
						int32_t x = pop32();
						int32_t y = pop32();
						int32_t z = pop32();
						int32_t a = pop32();
						int32_t b = pop32();
						steppers::definePosition(Point(x,y,z,a,b));
					}
				} else if (command == HOST_CMD_DELAY) {
					if (command_buffer.getLength() >= 5) {
						mode = DELAY;
						command_buffer.pop(); // remove the command code
						// parameter is in milliseconds; timeouts need microseconds
						uint32_t microseconds = pop32() * 1000;
						delay_timeout.start(microseconds);
					}
				} else if (command == HOST_CMD_FIND_AXES_MINIMUM ||
						command == HOST_CMD_FIND_AXES_MAXIMUM) {
					if (command_buffer.getLength() >= 8) {
						command_buffer.pop(); // remove the command
						uint8_t flags = pop8();
						uint32_t feedrate = pop32(); // feedrate in us per step
						uint16_t timeout_s = pop16();
						bool direction = command == HOST_CMD_FIND_AXES_MAXIMUM;
						mode = HOMING;
						homing_timeout.start(timeout_s * 1000L * 1000L);
						steppers::startHoming(command==HOST_CMD_FIND_AXES_MAXIMUM,
								flags,
								feedrate);
					}
				} else if (command == HOST_CMD_WAIT_FOR_TOOL) {
					if (command_buffer.getLength() >= 6) {
						mode = WAIT_ON_TOOL;
						command_buffer.pop();
						uint8_t currentToolIndex = command_buffer.pop();
						uint16_t toolPingDelay = (uint16_t)pop16();
						uint16_t toolTimeout = (uint16_t)pop16();
						tool_wait_timeout.start(toolTimeout*1000000L);
					}
				} else if (command == HOST_CMD_WAIT_FOR_PLATFORM) {
	        // FIXME: Almost equivalent to WAIT_FOR_TOOL
					if (command_buffer.getLength() >= 6) {
						mode = WAIT_ON_PLATFORM;
						command_buffer.pop();
						uint8_t currentToolIndex = command_buffer.pop();
						uint16_t toolPingDelay = (uint16_t)pop16();
						uint16_t toolTimeout = (uint16_t)pop16();
						tool_wait_timeout.start(toolTimeout*1000000L);
					}
				} else if (command == HOST_CMD_STORE_HOME_POSITION) {

					// check for completion
					if (command_buffer.getLength() >= 2) {
						command_buffer.pop();
						uint8_t axes = pop8();

						// Go through each axis, and if that axis is specified, read it's value,
						// then record it to the eeprom.
						for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
							if ( axes & (1 << i) ) {
							eeprom_address(AXIS_HOME_POSITIONS + (i*4)) = steppers::getPosition()[i];
						}
						save_to_flash();
					}
				}
			} else if (command == HOST_CMD_RECALL_HOME_POSITION) {
				// check for completion
				if (command_buffer.getLength() >= 2) {
					command_buffer.pop();
					uint8_t axes = pop8();

					Point newPoint = steppers::getPosition();

					for (uint8_t i = 0; i < STEPPER_COUNT; i++) {
						if ( axes & (1 << i) ) {
							newPoint[i] = eeprom_address(AXIS_HOME_POSITIONS + (i*4));
						}
					}
					steppers::definePosition(newPoint);
				}

			} else if (command == HOST_CMD_TOOL_COMMAND) {
				if (command_buffer.getLength() >= 4) { // needs a payload
					uint8_t payload_length = command_buffer[3];
					if (command_buffer.getLength() >= 4+payload_length) {
						// command is ready
						if (tool::getLock()) {
							OutPacket& out = tool::getOutPacket();
							out.reset();
							command_buffer.pop(); // remove the command code
							out.append8(command_buffer.pop()); // copy tool index
							out.append8(command_buffer.pop()); // copy command code
							int len = pop8(); // get payload length
							for (int i = 0; i < len; i++) {
								out.append8(command_buffer.pop());
							}
							// we don't care about the response, so we can release
							// the lock after we initiate the transfer
							tool::startTransaction();
							tool::releaseLock();
						}
					}
				}
			} else {
			}
		}
	}
}
}
