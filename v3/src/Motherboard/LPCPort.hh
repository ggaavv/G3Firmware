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

#ifndef SHARED_AVR_PORT_HH_
#define SHARED_AVR_PORT_HH_

extern "C" {
	#include "lpc17xx_gpio.h"
}

// The AVR port and pin mapping is based on a convention that has held true for all ATMega chips
// released so far: that the ports begin in sequence from register 0x00 from A onwards, and are
// arranged:
// 0 PINx
// 1 DDRx
// 2 PORTx
// This is verified true for the 168/644p/1280.

/* GPIO style -------------------------------
void GPIO_SetDir(uint8_t portNum, uint32_t bitValue, uint8_t dir);
void GPIO_SetValue(uint8_t portNum, uint32_t bitValue);
void GPIO_ClearValue(uint8_t portNum, uint32_t bitValue);
uint32_t GPIO_ReadValue(uint8_t portNum);
void GPIO_IntCmd(uint8_t portNum, uint32_t bitValue, uint8_t edgeState);
FunctionalState GPIO_GetIntStatus(uint8_t portNum, uint32_t pinNum, uint8_t edgeState);
void GPIO_ClearInt(uint8_t portNum, uint32_t bitValue);
*/

//#if defined Eight_Sixteen_bit
//typedef uint16_t port_base_t;
//#define NULL_PORT 0xffff
//#else
typedef uint8_t port_base_t;
#define NULL_PORT 0xff
//#endif

class Port {
private:
	port_base_t port_base;
public:
	Port() : port_base(NULL_PORT) {}
	Port(port_base_t port_base_in) : port_base(port_base_in) {}
	bool isNull() { return port_base == NULL_PORT; }
	void setPinDirection(uint8_t pin_index, bool out) {
		GPIO_SetDir(port_base, 1 << pin_index, out);
	}
	bool getPin(uint8_t pin_index) {
		return (((uint32_t)GPIO_ReadValue(port_base) & (1 << pin_index)) != 0);
	}
	void setPin(uint8_t pin_index, bool on) {
		GPIO_SetValue(port_base, ((GPIO_ReadValue(port_base)&(1 << pin_index)) & (1 << pin_index)) | (on?(1 << pin_index):0));
	}
};

extern Port Port0, Port1, Port2, Port3, Port4;

class Pin {
private:
	Port port;
	uint8_t pin_index : 8; // Bit Field
public:
	Pin() : port(Port()), pin_index(0) {}
	Pin(Port port_in, uint8_t pin_index_in) : port(port), pin_index(pin_index_in) {}
	bool isNull() { return port.isNull(); }
	void setDirection(bool out) { port.setPinDirection(pin_index,out); }
	bool getValue() { return port.getPin(pin_index); }
	void setValue(bool on) { port.setPin(pin_index,on); }
	const uint8_t getPinIndex() const { return pin_index; }
};

#endif // SHARED_AVR_PORT_HH_

