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
#include "Microsteps.hh"
#include "Version.hh"
#include "IAP.hh"
#include "LPC17xx.h"

void read_all_from_flash (void){
	__disable_irq ();
	for (uint32_t i = 0; i < (USER_FLASH_AREA_SIZE); i++) {
//		*((uint32_t*)eeprom::EEPROM_START_ADDRESS + i) = *((uint32_t*)USER_FLASH_AREA_START + i);
		eeprom_address(EEPROM_START_ADDRESS+i) = eeprom_address(USER_FLASH_AREA_START + i);
	}
	__enable_irq ();
};

void save_to_flash (void) {
	__disable_irq ();
	IAP in_ap_prog;
	int error_code_ret = in_ap_prog.erase(USER_FLASH_AREA_START, USER_FLASH_AREA_START);
	error_code_ret = in_ap_prog.write((char)0x10000000, (char)USER_FLASH_AREA_START, (int)USER_FLASH_AREA_SIZE );
	// read all variables back into Ram
	__enable_irq ();
};

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
		eeprom_address(MICROSTEPS_P0) = microsteps::microstep_pinout(0);
		eeprom_address(MICROSTEPS_P1) = microsteps::microstep_pinout(1);
		eeprom_address(AXIS_INVERSION) = axis_invert;
		eeprom_address(ENDSTOP_INVERSION) = endstop_invert;
		eeprom_address(MACHINE_NAME) = 0; // name is null
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
