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

#include "Host.hh"
#include "Command.hh"
#include "Tool.hh"
#include "Commands.hh"
#include "Steppers.hh"
#include "DebugPacketProcessor.hh"
#include "Timeout.hh"
#include "Version.hh"
//#include <util/atomic.h>
#include "Atomic.hh"
//#include <avr/eeprom.h>
#include "pgmspace.h"
#include "Main.hh"
#include "Errors.hh"
#include "EepromMap.hh"
/********************************/
//#include "test.hh"  // testing
//#include "test_led.hh"  // testing
//#include "test_u.hh"
#include "Delay.hh"
//	#include "lpc17xx_nvic.h"
//	#include "lpc17xx_timer.h"
//	#include "LPC17xx.h"
//test_led(1);
#include "Uart32.h"
/********************************/

namespace host {

/// Identify a command packet, and process it.  If the packet is a command
/// packet, return true, indicating that the packet has been queued and no
/// other processing needs to be done. Otherwise, processing of this packet
/// should drop through to the next processing level.
bool processCommandPacket(const InPacket& from_host, OutPacket& to_host);
bool processQueryPacket(const InPacket& from_host, OutPacket& to_host);

// Timeout from time first bit recieved until we abort packet reception
Timeout packet_in_timeout;

#define HOST_PACKET_TIMEOUT_MS 20
#define HOST_PACKET_TIMEOUT_MICROS (1000L*HOST_PACKET_TIMEOUT_MS)

#define HOST_TOOL_RESPONSE_TIMEOUT_MS 50
#define HOST_TOOL_RESPONSE_TIMEOUT_MICROS (1000L*HOST_TOOL_RESPONSE_TIMEOUT_MS)

char machineName[MAX_MACHINE_NAME_LEN];

char buildName[MAX_FILE_LEN];

uint32_t buildSteps;

HostState currentState;

bool do_host_reset = true;

void runHostSlice() {
	InPacket& in = UART::getHostUART().in;
	OutPacket& out = UART::getHostUART().out;
//	UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x1111);
	if (out.isSending()) {
		// still sending; wait until send is complete before reading new host packets.
		return;
	}
//	UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x2222);
	if (do_host_reset) {
		do_host_reset = false;
		// Then, reset local board
		reset(false);
		packet_in_timeout.abort();

		// Clear the machine and build names
		machineName[0] = 0;
		buildName[0] = 0;
		currentState = HOST_STATE_READY;

		return;
	}
//	UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x3333);
	if (in.isStarted() && !in.isFinished()) {
		if (!packet_in_timeout.isActive()) {
			// initiate timeout
			packet_in_timeout.start(HOST_PACKET_TIMEOUT_MICROS);
		} else if (packet_in_timeout.hasElapsed()) {
			in.timeout();
		}
	}
//	UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x4444);
	if (in.hasError()) {
		// Reset packet quickly and start handling the next packet.
		// Report error code.
		if (in.getErrorCode() == PacketError::PACKET_TIMEOUT) {
			Motherboard::getBoard().indicateError(ERR_HOST_PACKET_TIMEOUT);
		} else {
			Motherboard::getBoard().indicateError(ERR_HOST_PACKET_MISC);
		}
		in.reset();
	}
//	UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x4444);
	if (in.isFinished()) {
//		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x1111);
		packet_in_timeout.abort();
		out.reset();
//		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x2222);
#if defined(HONOR_DEBUG_PACKETS) && (HONOR_DEBUG_PACKETS == 1)
		if (processDebugPacket(in, out)) {
			// okay, processed
		} else
//		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x3333);
#endif
		if (processCommandPacket(in, out)) {
//			UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x4444);
			// okay, processed
		} else if (processQueryPacket(in, out)) {
//			UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x5555);
			// okay, processed
		} else {
			// Unrecognized command
			out.append8(RC_CMD_UNSUPPORTED);
//			UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x6666);
		}
		in.reset();
		UART::getHostUART().beginSend();
//		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x7777);
	}
//	UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x5555);
}

/// Identify a command packet, and process it.  If the packet is a command
/// packet, return true, indicating that the packet has been queued and no
/// other processing needs to be done. Otherwise, processing of this packet
/// should drop through to the next processing level.
bool processCommandPacket(const InPacket& from_host, OutPacket& to_host) {
//	UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x1111);
	if (from_host.getLength() >= 1) {
//		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x2222);
		uint8_t command = from_host.read8(0);
		if ((command & 0x80) != 0) {
//		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x3333);
			// If we're capturing a file to an SD card, we send it to the sdcard module
			// for processing.
//			UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x4444);
//			_delay_ms(100);
			if (sdcard::isCapturing()) {
				sdcard::capturePacket(from_host);
				to_host.append8(RC_OK);
				return true;
			}
//			UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x5555);
//			_delay_ms(100);
			// Queue command, if there's room.
			// Turn off interrupts while querying or manipulating the queue!
//			ATOMIC_BLOCK(ATOMIC_FORCEON) {
//			__disable_irq ();
			Atomic(BEGIN_INT);
			const uint8_t command_length = from_host.getLength();
			if (command::getRemainingCapacity() >= command_length) {
//				UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x6666);
				// Append command to buffer
//				UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x8888);
				for (int i = 0; i < command_length; i++) {
//					UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x7777);
					command::push(from_host.read8(i));
//					UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x8888);
				}
				to_host.append8(RC_OK);
//				UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x9999);
			} else {
				to_host.append8(RC_BUFFER_OVERFLOW);
//				UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x1222);
			}
//			__enable_irq ();
			Atomic(RESTORE_INT);
//			UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x1333);
//			}
			return true;
		}
	}
	return false;
}
/*
//checks version numbers to make sure we (mobo firmware) can work with the
// host driver.
bool void handleVersion2(uint16 host_driver_version) {

    //new firmware cannot work with host software version 25 or older
    // lie about our version number so we can get an good error
    if(host_driver_version <= 25) {
       // to_host.append8(RC_GENERIC_ERROR);
        //to_host.append16(0000);
    }
}*/


// Received driver version info, and request for fw version info.
// puts fw version into a reply packet, and send it back
inline void handleVersion(const InPacket& from_host, OutPacket& to_host) {

    // Case to give an error on Replicator G versions older than 0025. See footnote 1
    if(from_host.read16(1)  <=  25   ) {
        to_host.append8(RC_OK);
        to_host.append16(0x0000);
    }
    else  {
        to_host.append8(RC_OK);
//        UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, firmware_version);
        to_host.append16(firmware_version);  //  0x0140 = 3.20
    }

}

inline void handleGetBuildName(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	for (uint8_t idx = 0; idx < 31; idx++) {
	  to_host.append8(buildName[idx]);
	  if (buildName[idx] == '\0') { break; }
	}
}

inline void handleGetBufferSize(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	to_host.append32(command::getRemainingCapacity());
}

inline void handleGetPosition(const InPacket& from_host, OutPacket& to_host) {
//	ATOMIC_BLOCK(ATOMIC_FORCEON) {
//	__disable_irq ();
	Atomic(BEGIN_INT);
	const Point p = steppers::getPosition();
	to_host.append8(RC_OK);
	to_host.append32(p[0]);
	to_host.append32(p[1]);
	to_host.append32(p[2]);
	// From spec:
	// endstop status bits: (7-0) : | N/A | N/A | z max | z min | y max | y min | x max | x min |
	Motherboard& board = Motherboard::getBoard();
	uint8_t endstop_status = 0;
	for (int i = 3; i > 0; i--) {
		StepperInterface& si = board.getStepperInterface(i-1);
		endstop_status <<= 2;
		endstop_status |= (si.isAtMaximum()?2:0) | (si.isAtMinimum()?1:0);
	}
	to_host.append8(endstop_status);
//	__enable_irq ();
	Atomic(RESTORE_INT);
//	}
}

inline void handleGetPositionExt(const InPacket& from_host, OutPacket& to_host) {
//	ATOMIC_BLOCK(ATOMIC_FORCEON) {
//	__disable_irq ();
//	Atomic(BEGIN_INT);
	const Point p = steppers::getPosition();
	to_host.append8(RC_OK);
	to_host.append32(p[0]);
	to_host.append32(p[1]);
	to_host.append32(p[2]);
#if STEPPER_COUNT > 3
	to_host.append32(p[3]);
	to_host.append32(p[4]);
#else
	to_host.append32(0);
	to_host.append32(0);
#endif
	// From spec:
	// endstop status bits: (15-0) : | b max | b min | a max | a min | z max | z min | y max | y min | x max | x min |
	Motherboard& board = Motherboard::getBoard();
	uint8_t endstop_status = 0;
	for (int i = STEPPER_COUNT; i > 0; i--) {
		StepperInterface& si = board.getStepperInterface(i-1);
		endstop_status <<= 2;
		endstop_status |= (si.isAtMaximum()?2:0) | (si.isAtMinimum()?1:0);
	}
	to_host.append16(endstop_status);
//	__enable_irq ();
//	Atomic(RESTORE_INT);
//	}
}

inline void handleCaptureToFile(const InPacket& from_host, OutPacket& to_host) {
	char *p = (char*)from_host.getData() + 1;
	to_host.append8(RC_OK);
	to_host.append8(sdcard::startCapture(p));
}

inline void handleEndCapture(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	to_host.append32(sdcard::finishCapture());
}

inline void handlePlayback(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	for (int idx = 1; idx < from_host.getLength(); idx++) {
		buildName[idx-1] = from_host.read8(idx);
	}
	buildName[MAX_FILE_LEN-1] = '\0';

	uint8_t response = startBuildFromSD();
	to_host.append8(response);
}

inline void handleNextFilename(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	uint8_t resetFlag = from_host.read8(1);
	if (resetFlag != 0) {
		sdcard::SdErrorCode e = sdcard::directoryReset();
		if (e != sdcard::SD_SUCCESS) {
			to_host.append8(e);
			to_host.append8(0);
			return;
		}
	}
	const int MAX_FILE_LEN = MAX_PACKET_PAYLOAD-1;
	char fnbuf[MAX_FILE_LEN];
	sdcard::SdErrorCode e;
	// Ignore dot-files
	do {
		e = sdcard::directoryNextEntry(fnbuf,MAX_FILE_LEN);
		if (fnbuf[0] == '\0') break;
	} while (e == sdcard::SD_SUCCESS && fnbuf[0] == '.');
	to_host.append8(e);
	uint8_t idx;
	for (idx = 0; (idx < MAX_FILE_LEN) && (fnbuf[idx] != 0); idx++) {
		to_host.append8(fnbuf[idx]);
	}
	to_host.append8(0);
}

void doToolPause(OutPacket& to_host) {
	Timeout acquire_lock_timeout;
	acquire_lock_timeout.start(HOST_TOOL_RESPONSE_TIMEOUT_MS);
	while (!tool::getLock()) {
		if (acquire_lock_timeout.hasElapsed()) {
			to_host.append8(RC_DOWNSTREAM_TIMEOUT);
			Motherboard::getBoard().indicateError(ERR_SLAVE_LOCK_TIMEOUT);
			return;
		}
	}
	OutPacket& out = tool::getOutPacket();
	InPacket& in = tool::getInPacket();
	out.reset();
	out.append8(tool::getCurrentToolheadIndex());
	out.append8(SLAVE_CMD_PAUSE_UNPAUSE);
	// Timeouts are handled inside the toolslice code; there's no need
	// to check for timeouts on this loop.
	tool::startTransaction();
	tool::releaseLock();
	// WHILE: bounded by tool timeout in runToolSlice
	while (!tool::isTransactionDone()) {
		tool::runToolSlice();
	}
	if (in.getErrorCode() == PacketError::PACKET_TIMEOUT) {
		to_host.append8(RC_DOWNSTREAM_TIMEOUT);
	} else {
		// Copy payload back. Start from 0-- we need the response code.
		for (int i = 0; i < in.getLength(); i++) {
			to_host.append8(in.read8(i));
		}
	}
}

inline void handleToolQuery(const InPacket& from_host, OutPacket& to_host) {
	// Quick sanity assert: ensure that host packet length >= 2
	// (Payload must contain toolhead address and at least one byte)
	if (from_host.getLength() < 2) {
		to_host.append8(RC_GENERIC_ERROR);
		Motherboard::getBoard().indicateError(ERR_HOST_TRUNCATED_CMD);
		return;
	}
	Timeout acquire_lock_timeout;
	acquire_lock_timeout.start(HOST_TOOL_RESPONSE_TIMEOUT_MS);
	while (!tool::getLock()) {
		if (acquire_lock_timeout.hasElapsed()) {
			to_host.append8(RC_DOWNSTREAM_TIMEOUT);
			Motherboard::getBoard().indicateError(ERR_SLAVE_LOCK_TIMEOUT);
			return;
		}
	}
	OutPacket& out = tool::getOutPacket();
	InPacket& in = tool::getInPacket();
	out.reset();
	for (int i = 1; i < from_host.getLength(); i++) {
		out.append8(from_host.read8(i));
	}
	// Timeouts are handled inside the toolslice code; there's no need
	// to check for timeouts on this loop.
	tool::startTransaction();
	tool::releaseLock();
	// WHILE: bounded by tool timeout in runToolSlice
	while (!tool::isTransactionDone()) {
		tool::runToolSlice();
	}
	if (in.getErrorCode() == PacketError::PACKET_TIMEOUT) {
		to_host.append8(RC_DOWNSTREAM_TIMEOUT);
	} else {
		// Copy payload back. Start from 0-- we need the response code.
		for (int i = 0; i < in.getLength(); i++) {
			to_host.append8(in.read8(i));
		}
	}
}

inline void handlePause(const InPacket& from_host, OutPacket& to_host) {
	command::pause(!command::isPaused());
	doToolPause(to_host);
	to_host.append8(RC_OK);
}

inline void handleIsFinished(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
//	ATOMIC_BLOCK(ATOMIC_FORCEON) {
//	__disable_irq ();
	Atomic(BEGIN_INT);
	bool done = !steppers::isRunning() && command::isEmpty();
	to_host.append8(done?1:0);
//	__enable_irq ();
	Atomic(RESTORE_INT);
//	}
}

inline void handleReadEeprom(const InPacket& from_host, OutPacket& to_host) {
	uint16_t offset = from_host.read16(1);
	uint8_t length = from_host.read8(3);
	to_host.append8(RC_OK);
	for (int i = 0; i < length; i++) {
		to_host.append8(eeprom_address(EEPROM_START_ADDRESS + offset + i));
//		UART_8((LPC_UART_TypeDef *)LPC_UART2, 7);
//		UART_8((LPC_UART_TypeDef *)LPC_UART2, (eeprom_address(EEPROM_START_ADDRESS + offset + i)));
//		UART_8((LPC_UART_TypeDef *)LPC_UART2, 7);
//		UART_8((LPC_UART_TypeDef *)LPC_UART2, 0xa);
	}
}

inline void handleWriteEeprom(const InPacket& from_host, OutPacket& to_host) {
	uint16_t offset = from_host.read16(1);
	uint8_t length = from_host.read8(3);
	for (int i = 0; i < length; i++) {
		eeprom_address(EEPROM_START_ADDRESS + offset + i) = from_host.read8(i + 4);
//		UART_8((LPC_UART_TypeDef *)LPC_UART2, 8);
//		UART_8((LPC_UART_TypeDef *)LPC_UART2, (eeprom_address(EEPROM_START_ADDRESS + offset + i)));
//		UART_8((LPC_UART_TypeDef *)LPC_UART2, 8);
//		UART_8((LPC_UART_TypeDef *)LPC_UART2, 0xa);
	}
	save_to_flash();
	to_host.append8(RC_OK);
	to_host.append8(length);
}

enum { // bit assignments
	ES_STEPPERS = 0, // stop steppers
	ES_COMMANDS = 1  // clean queue
};

inline void handleExtendedStop(const InPacket& from_host, OutPacket& to_host) {
	uint8_t flags = from_host.read8(1);
	if (flags & (1 << (ES_STEPPERS))) {
		steppers::abort();
	}
	if (flags & (1 << (ES_COMMANDS))) {
		command::reset();
	}
	to_host.append8(RC_OK);
	to_host.append8(0);
}

//inline void handleBuildStartNotification(const InPacket& from_host, OutPacket& to_host) {
//	uint8_t flags = from_host.read8(1);
//
//	buildSteps = from_host.read32(1);
//
//	for (int idx = 4; idx < from_host.getLength(); idx++) {
//		buildName[idx-4] = from_host.read8(idx);
//	}
//	buildName[MAX_FILE_LEN-1] = '\0';
//
//	currentState = HOST_STATE_BUILDING;
//
//	to_host.append8(RC_OK);
//}


inline void handleGetCommunicationStats(const InPacket& from_host, OutPacket& to_host) {
	to_host.append8(RC_OK);
	to_host.append32(0);
	to_host.append32(tool::getSentPacketCount());
	to_host.append32(tool::getPacketFailureCount());
	to_host.append32(tool::getRetryCount());
	to_host.append32(tool::getNoiseByteCount());
}

bool processQueryPacket(const InPacket& from_host, OutPacket& to_host) {
	if (from_host.getLength() >= 1) {
		uint8_t command = from_host.read8(0);
		if ((command & 0x80) == 0) {
			// Is query command.
			switch (command) {
			case HOST_CMD_VERSION:
				handleVersion(from_host,to_host);
				return true;
			case HOST_CMD_GET_BUILD_NAME:
				handleGetBuildName(from_host,to_host);
				return true;
			case HOST_CMD_INIT:
				// There's really nothing we want to do here; we don't want to
				// interrupt a running build, for example.
				to_host.append8(RC_OK);
				return true;
			case HOST_CMD_CLEAR_BUFFER: // equivalent at current time
			case HOST_CMD_ABORT: // equivalent at current time
			case HOST_CMD_RESET:
				// TODO: This is fishy.
				if (currentState == HOST_STATE_BUILDING
						|| currentState == HOST_STATE_BUILDING_FROM_SD) {
					stopBuild();
				}

				do_host_reset = true; // indicate reset after response has been sent
				to_host.append8(RC_OK);
				return true;
			case HOST_CMD_GET_BUFFER_SIZE:
				handleGetBufferSize(from_host,to_host);
				return true;
			case HOST_CMD_GET_POSITION:
				handleGetPosition(from_host,to_host);
				return true;
			case HOST_CMD_GET_POSITION_EXT:
				handleGetPositionExt(from_host,to_host);
				return true;
			case HOST_CMD_CAPTURE_TO_FILE:
				handleCaptureToFile(from_host,to_host);
				return true;
			case HOST_CMD_END_CAPTURE:
				handleEndCapture(from_host,to_host);
				return true;
			case HOST_CMD_PLAYBACK_CAPTURE:
				handlePlayback(from_host,to_host);
				return true;
			case HOST_CMD_NEXT_FILENAME:
				handleNextFilename(from_host,to_host);
				return true;
			case HOST_CMD_GET_RANGE:
			case HOST_CMD_SET_RANGE:
				break; // not yet implemented
			case HOST_CMD_PAUSE:
				handlePause(from_host,to_host);
				return true;
			case HOST_CMD_TOOL_QUERY:
				handleToolQuery(from_host,to_host);
				return true;
			case HOST_CMD_IS_FINISHED:
				handleIsFinished(from_host,to_host);
				return true;
			case HOST_CMD_READ_EEPROM:
				handleReadEeprom(from_host,to_host);
				return true;
			case HOST_CMD_WRITE_EEPROM:
				handleWriteEeprom(from_host,to_host);
				return true;
			case HOST_CMD_EXTENDED_STOP:
				handleExtendedStop(from_host,to_host);
				return true;
//			case HOST_CMD_BUILD_START_NOTIFICATION:
//				handleBuildStartNotification(from_host,to_host);
//				return true;
//			case HOST_CMD_BUILD_STOP_NOTIFICATION	:
//				handleBuildStopNotification(from_host,to_host);
//				return true;
			case HOST_CMD_GET_COMMUNICATION_STATS:
				handleGetCommunicationStats(from_host,to_host);
				return true;
			}
		}
	}
	return false;
}

char* getMachineName() {
	// If the machine name hasn't been loaded, load it
	if (machineName[0] == 0) {
		for(uint8_t i = 0; i < MAX_MACHINE_NAME_LEN; i++) {
			machineName[i] = eeprom_address(MACHINE_NAME + i) & 0xff;
		}
	}

	// If it's still zero, load in a default.
	static PROGMEM char defaultMachineName[] =  "Thing-O-Matic";

	if (machineName[0] == 0) {
		for(uint8_t i = 0; i < 14; i++) {
			machineName[i] = defaultMachineName[i];
		}
	}
	save_to_flash ();
	return machineName;
}

char* getBuildName() {
	return buildName;
}

HostState getHostState() {
	return currentState;
}

sdcard::SdErrorCode startBuildFromSD() {
	sdcard::SdErrorCode e;

	// Attempt to start build
	e = sdcard::startPlayback(buildName);
	if (e != sdcard::SD_SUCCESS) {
		// TODO: report error
		return e;
	}

	currentState = HOST_STATE_BUILDING_FROM_SD;

	return e;
}

// Stop the current build, if any
void stopBuild() {
	do_host_reset = true; // indicate reset after response has been sent
}

}

/* footnote 1: due to a protocol change, replicatiorG 0026 and newer can ONLY work with
 * firmware 3.00 and newer. Because replicatorG handles version mismatches poorly,
 * if our firmware is 3.0 or newer, *AND* the connecting replicatorG is 25 or older, we
 * lie, and reply with firmware 0.00 to case ReplicatorG to display a 'null version' error
 * so users will know to upgrade.
 */
