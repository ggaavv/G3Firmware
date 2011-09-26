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
FIO_SetMask(uint8_t portNum, uint32_t bitValue, uint8_t maskValue)
*/

enum portname {
	Port0 = 0x00,
	Port1 = 0x01,
	Port2 = 0x02,
	Port3 = 0x03,
	Port4 = 0x04,
	NULL_PORT = 0xff
};

//#if defined Eight_Sixteen_bit
//typedef uint16_t port_base_t;
//#define NULL_PORT 0xffff
//#else
typedef uint8_t port_base_t;
//#endif

class Port_Class {
private:
	portname port_base;
public:
	Port_Class() : port_base(NULL_PORT) {}
	Port_Class(portname port_base_in) : port_base(port_base_in) {}
	bool isNull() { return port_base == NULL_PORT; }
	void setPinDirection(uint8_t pin_index, bool out) {
		uint32_t getpindirin = GPIO_GetDir(port_base);
		if (out){
			GPIO_SetDir(port_base, (getpindirin | (1 << pin_index)), out);
		}else {
			GPIO_SetDir(port_base, (getpindirin & ~(1 << pin_index)), out);
		}
	}
	bool getPin(uint8_t pin_index) {
		FIO_SetMask(port_base, ~(1 << pin_index), 1);
		return (GPIO_ReadValue(port_base));
	}
	void setPin(uint8_t pin_index, bool on) {
		FIO_SetMask(port_base, ~(1 << pin_index), 1);
		if (on){
			GPIO_ClearValue(port_base, 0xffffffff);
		}else{
			GPIO_SetValue(port_base, 0xffffffff);
		}
	}
};

//extern Port Port0, Port1, Port2, Port3, Port4;
/*
Port Port0(0x00);
Port Port1(0x01);
Port Port2(0x02);
Port Port3(0x03);
Port Port4(0x04);
*/

Port_Class port;

class Pin {
private:
	Port_Class port;
	portname Port;
	uint8_t pin_index;
public:
	Pin() : port(Port), pin_index(0) {}
	Pin(portname port_in, uint8_t pin_index_in) : port(port_in), pin_index(pin_index_in) {}
	bool isNull() { return port.isNull(); }
	void setDirection(bool out) { port.setPinDirection(pin_index,out); }
	bool getValue() { return port.getPin(pin_index); }
	void setValue(bool on) { port.setPin(pin_index,on); }
	uint8_t getPinIndex() { return pin_index; }
};

#endif // SHARED_AVR_PORT_HH_

