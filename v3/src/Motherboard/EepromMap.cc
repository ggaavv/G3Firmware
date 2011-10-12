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
#include "Atomic.hh"
/********************************/
#include "test.hh"  // testing
#include "test_led.hh"  // testing
#include "test_u.hh"
#include "Delay.hh"
//	#include "lpc17xx_timer.h"
//	#include "LPC17xx.h"
//	#include "lpc17xx_clkpwr.h"
//test_led(1);
#include "Uart32.h"
/********************************/

void print_eeprom (void){
	for (uint32_t i = 0; i < 0x150; i += 4) {
		uint8_t eemenu222[] = "\na:";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, eemenu222, sizeof(eemenu222), BLOCKING);
		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, EEPROM_FLASH_AREA_START + i);

		uint8_t eemenu2722[] = " fd:";
		UART_Send((LPC_UART_TypeDef *)LPC_UART2, eemenu2722, sizeof(eemenu2722), BLOCKING);
		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, eeprom_address(EEPROM_FLASH_AREA_START + i));
	}
}

void read_all_from_flash (void){
	Atomic(BEGIN_INT);
	uint32_t i;
	for (i = 0; i < EEPROM_SIZE; i=i+4) {
		eeprom_address(EEPROM_START_ADDRESS+i) = eeprom_address(EEPROM_FLASH_AREA_START + i);
	}
	Atomic(RESTORE_INT);
};

void save_to_flash (void) {
	Atomic(BEGIN_INT);
	IAP in_ap_prog;
#define	TARGET_SECTOR	2 // Flash Sector Number 2 = 0x0002000
	in_ap_prog.prepare( TARGET_SECTOR, TARGET_SECTOR );
	int error_code_erase = in_ap_prog.erase(TARGET_SECTOR, TARGET_SECTOR);
	in_ap_prog.prepare( TARGET_SECTOR, TARGET_SECTOR );
	int error_code_write = in_ap_prog.write((char*)EEPROM_START_ADDRESS, (char*)USER_FLASH_AREA_START, 4096 );
	// read all variables back into Ram
	Atomic(RESTORE_INT);
};

//	[STEPPER_COUNT][MSx][port, no, MSx]
uint8_t microstep_pinout(uint8_t port_no) {
	uint8_t microsteping_port;
	for (uint8_t ii = 0; ii < 5; ii++){   //Counts through STEPPER_COUNT
		for (uint8_t iii = 0; iii < 3; iii++){
			uint8_t port_bit = 1 << microstep_port_array[ii][iii][1];
			if (microstep_port_array[ii][iii][0] == port_no) {
				microsteping_port = microsteping_port | (microstep_port_array[ii][iii][2] << port_bit);
			}
		}
	}
	UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, microsteping_port);
	return microsteping_port;
}

namespace eeprom {

void init() {
	Atomic(DISABLE_INT);
//	print_eeprom();
	read_all_from_flash();
	uint8_t version[2];
	version[0] = eeprom_address(VERSION_LOW);
	version[1] = eeprom_address(VERSION_HIGH);
	UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, 0x1234);
	if ((version[1]*100+version[0]) == firmware_version) return;
	if (version[1] == 0xff || version[1] < 2) {
		// Initialise eeprom map
		// Default: enstops inverted, Y axis inverted
								//		 bazyx
		eeprom_address(AXIS_INVERSION) = 	0b00000011; // 1<<1; // Y axis = 1
//		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, axis_inversion[0]);
									//		 bazyx
		eeprom_address(ENDSTOP_INVERSION) = 0b00010111; // all endstops inverted
//		UART_32_HEX((LPC_UART_TypeDef *)LPC_UART2, endstop_inversion[0]);

		eeprom_address(MICROSTEPS_P0) = microstep_pinout(0);
		eeprom_address(MICROSTEPS_P1) = microstep_pinout(1);
		for (uint8_t i = 0; i < 100; i++) {
			eeprom_address(MACHINE_NAME+i) = 0x00; // name is null
		}
	}
	// Write version
	version[0] = firmware_version % 100;
	version[1] = firmware_version / 100;
	eeprom_address(VERSION_LOW) = version[0];
	eeprom_address(VERSION_HIGH) = version[1];
	save_to_flash();
//	print_eeprom();
	Atomic(ENABLE_INT);
}

uint8_t getEeprom8(uint32_t location, const uint8_t default_value) {
	uint8_t data;
	location = location*4;
	data = *(uint32_t*)location;
	if (data == 0xff) data = default_value;
	return data;
}

uint16_t getEeprom16(uint32_t location, const uint16_t default_value) {
	uint16_t data;
	location = location*4;
	data = *(uint32_t*)location;
	if (data == 0xffff) data = default_value;
	return data;
}

} // namespace eeprom
