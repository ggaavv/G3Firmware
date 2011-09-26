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

#ifndef UART_H_
#define UART_H_

#include "Packet.hh"
#include "Configuration.hh"
#include <stdint.h>

/**
 * UARTs, when constructed, start off disabled.
 * They begin receiving data only after an enable(true)
 * call is made.  beginSend() calls will send completed
 * packets.
 *
 */
/*
enum uart_no {
	Host_Uart = 0x00,
	Slave_Uart = 0x01
};
*/
//uint8_t Host_Uart = 0;
//uint8_t Slave_Uart = 1;

class UART {
private:
	uint8_t index_;
	volatile bool enabled_;
public:
//	UART(uart_no index_) : index_(0), enabled_(false) {}
	UART();
	UART(uint8_t index);
//	UART(uint8_t index) {}// : index_(index), enabled_(false) {}
	void change_index(uint8_t index_);
	uint8_t read_index();
	InPacket in;
	OutPacket out;
	void beginSend();
	void enable(bool enabled);
	static UART uart[2];
	static UART& getHostUART() { return UART::uart[0]; }
	static UART& getSlaveUART() { return UART::uart[1]; }
	// Reset the UART to a listening state.  This is important for
	// RS485-based comms.
	void reset();	// Not meant to be public, but otherwise we'd have to friend interrupt protos.  :/
};

UART UART::uart[2] = {
	UART(0),
	UART(1),
};

#endif // UART_HH_
