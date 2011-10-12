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

/********************************/
//#include "test.hh"  // testing
//#include "test_led.hh"  // testing
#include "test_u.hh"
//#include "Delay.hh"
//	#include "lpc17xx_nvic.h"
//	#include "lpc17xx_timer.h"
//	#include "LPC17xx.h"
//test_led(1);
extern "C" {
	#include "Uart32.h"
}
/********************************/

/*
#include "LPCPort.hh"

Port Port0 = 0x00;
Port Port1 = 0x01;
Port Port2 = 0x02;
Port Port3 = 0x03;
Port Port4 = 0x04;

Port Port0(0x00);
Port Port1(0x01);
Port Port2(0x02);
Port Port3(0x03);
Port Port4(0x04);

port_base_t Port0 = 0x00;
port_base_t Port1 = 0x01;
port_base_t Port2 = 0x02;
port_base_t Port3 = 0x03;
port_base_t Port4 = 0x04;
*/
