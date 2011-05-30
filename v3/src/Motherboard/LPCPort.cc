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


#include "AvrPort.hh"

class Port {
private:
	port_base_t port_base;
public:
	Port() : port_base(NULL_PORT) {}
	Port(port_base_t port_base_in) : port_base(port_base_in) {}

	bool isNull() { return port_base == NULL_PORT; }
	void setPinDirection(uint8_t pin_index, bool out) {
		DDRx = (DDRx & ~_BV(pin_index)) | (out?_BV(pin_index):0);
	}
	bool getPin(uint8_t pin_index) {
		return (PINx & _BV(pin_index)) != 0;
	}
	void setPin(uint8_t pin_index, bool on) {
		PORTx = (PORTx & ~_BV(pin_index)) | (on?_BV(pin_index):0);
	}
};

class Pin {
private:
	Port port;
	uint32_t pin_index;
public:
	Pin() : port, pin_index(0) {}
	Pin(Port& port_in, uint32_t pin_index_in) : port(port_in), pin_index(0<<pin_index_in) {}
	bool isNull() { return port.isNull(); }
	void setDirection(bool out) { port.setPinDirection(0<<pin_index,out); }
	bool getValue() { return port.getPin(0<<pin_index); }
	void setValue(bool on) { port.setPin(0<<pin_index,on); }
	const uint8_t getPinIndex() const { return pin_index; }
};

Port Port1(0x1);
Port Port2(0x2);
Port Port3(0x3);
Port Port4(0x4);

Port Pin1(0x01);
Port Pin2(0x02);
Port Pin3(0x04);
Port Pin4(0x08);
Port Pin5(0x10);
Port Pin6(0x20);
Port Pin7(0x40);
Port Pin8(0x80);
Port Pin9(0x100);
Port Pin10(0x200);
Port Pin11(0x400);
Port Pin12(0x800);
Port Pin13(0x1000);
Port Pin14(0x2000);
Port Pin15(0x4000);
Port Pin16(0x8000);
Port Pin17(0x10000);
Port Pin18(0x20000);
Port Pin19(0x40000);
Port Pin20(0x80000);
Port Pin21(0x100000);
Port Pin22(0x2000);
Port Pin23(0x2000);


