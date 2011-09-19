/*
 * Copyright 2010 by Adam Mayer <adam@makerbot.com>
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


#include "EepromMap.hh"
//#include "Microsteps.hh"
#include "Version.hh"
#include "IAP.hh"
#include "LPC17xx.h"
/********************************/
#include "test.hh"  // testing
#include "test_led.hh"  // testing
#include "test_u.hh"
#include "Uart32.c"
#include "Delay.hh"
	#include "lpc17xx_timer.h"
	#include "LPC17xx.h"
	#include "lpc17xx_clkpwr.h"
//test_led(1);
/********************************/

void read_all_from_flash (void){
//	__disable_irq ();
	uint32_t i;
	for (i = 0x00000000; i < EEPROM_SIZE; i++,i++,i++,i++) {
		eeprom_address(EEPROM_START_ADDRESS+i) = eeprom_address(EEPROM_FLASH_AREA_START + i);
	}
		uint8_t menu22[] = "\ni value : ";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu22, sizeof(menu22), BLOCKING);
		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, i);
		uint8_t menu222[] = " from flash address : ";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu222, sizeof(menu222), BLOCKING);
		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, EEPROM_FLASH_AREA_START + i);
		uint8_t menu223[] = " eeprom address : ";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu223, sizeof(menu223), BLOCKING);
		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, EEPROM_START_ADDRESS + i);
//	__enable_irq ();
};

void save_to_flash (void) {
//	__disable_irq ();
	IAP in_ap_prog;
	int error_code_ret = in_ap_prog.erase(USER_FLASH_AREA_START, USER_FLASH_AREA_START);
	error_code_ret = in_ap_prog.write((char)0x10000000, (char)USER_FLASH_AREA_START, (int)USER_FLASH_AREA_SIZE );
	// read all variables back into Ram
//	__enable_irq ();
};

uint8_t microstep_pinout(uint8_t port_no) {
	uint8_t microsteping_port;
	for (uint8_t ii = 0; ii < 5; ii++){   //Counts through STEPPER_COUNT
		for (uint8_t iii = 0; ii < 3; iii++){
			uint8_t port_bit = 0x1 << microstep_port_array [ii][iii][1];
			if (microstep_port_array [ii][iii][0] == port_no) {
				microsteping_port += (microstep_port_array [ii][iii][2] << port_bit);
			}
		}
	}
	return microsteping_port;
}

namespace eeprom {

void init() {
	read_all_from_flash();
	uint8_t version[2];
	version[0] = eeprom_address(VERSION_LOW);
	version[1] = eeprom_address(VERSION_HIGH);
	if ((version[1]*100+version[0]) == firmware_version) return;
	if (version[1] == 0xff || version[1] < 2) {
		// Initialize eeprom map
		// Default: enstops inverted, Y axis inverted
							//		  bazyx
		uint8_t axis_invert = 	 0b00000011; // 1<<1; // Y axis = 1
							//		  bazyx
		uint8_t endstop_invert = 0b00010111; // all endstops inverted
							//		  bazyx
		eeprom_address(MICROSTEPS_P0) = microstep_pinout(0);
		eeprom_address(MICROSTEPS_P1) = microstep_pinout(1);
		eeprom_address(AXIS_INVERSION) = axis_invert;
		eeprom_address(ENDSTOP_INVERSION) = endstop_invert;
		for (uint32_t i = MACHINE_NAME; i < (MACHINE_NAME + (32*4)); i++, i++, i++, i++) {
			eeprom_address(MACHINE_NAME+i) = 0x00000000; // name is null
		}
	}
	// Write version
	version[0] = firmware_version % 100;
	version[1] = firmware_version / 100;
	save_to_flash();
}

uint8_t getEeprom8(uint32_t location, const uint8_t default_value) {
	uint8_t data;
	data = *(uint32_t*)location;
	if (data == 0xff) data = default_value;
	return data;
}

uint16_t getEeprom16(uint32_t location, const uint16_t default_value) {
	uint16_t data;
	data = *(uint32_t*)location;
	if (data == 0xffff) data = default_value;
	return data;
}

} // namespace eeprom
