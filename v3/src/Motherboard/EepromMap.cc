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
//#include <avr/eeprom.h>
#include "Version.hh"
#include "IAP.hh"

void read_all_from_flash (void){
	__enable_irq ();
	for (uint32_t i = USER_FLASH_AREA_START; i < USER_FLASH_AREA_SIZE; i++){
	*(EEPROM_START_ADDRESS + i) = *(USER_FLASH_AREA_START + i);
	__disable_irq ();
}

void save_to_flash (void) {
	__enable_irq ();
	IAP in_ap_prog;
	int error_code_ret = in_ap_prog.erase(USER_FLASH_AREA_START, USER_FLASH_AREA_START);
	error_code_ret = in_ap_prog.write((char)0x10000000, (char)USER_FLASH_AREA_START, (int)USER_FLASH_AREA_SIZE );
	// read all variables back into Ram
	__disable_irq ();
};

namespace eeprom {

void init() {

	uint8_t version[2];
	version[0] = *eeprom::VERSION_LOW;
	version[1] = *eeprom::VERSION_HIGH;
	if ((version[1]*100+version[0]) == firmware_version) return;
	if (version[1] == 0xff || version[1] < 2) {
		// Initialize eeprom map
		// Default: enstops inverted, Y axis inverted
		uint8_t axis_invert = 1<<1; // Y axis = 1
		uint8_t endstop_invert = 0b00010111; // all endstops inverted
		*eeprom::AXIS_INVERSION = axis_invert;
		*eeprom::ENDSTOP_INVERSION = endstop_invert;
		*eeprom::MACHINE_NAME = 0; // name is null
	}
	// Write version
	version[0] = firmware_version % 100;
	version[1] = firmware_version / 100;
	save_to_flash();
}

uint8_t getEeprom8(uint32_t *location, const uint8_t default_value) {
	uint8_t data;
	data = *location;
	if (data == 0xff) data = default_value;
	return data;
}

uint16_t getEeprom16(uint32_t *location, const uint16_t default_value) {
	uint16_t data;
	data = *location;
	if (data == 0xffff) data = default_value;
	return data;
}

} // namespace eeprom
