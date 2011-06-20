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

namespace eeprom {

/*
#define PREP_WR 50
#define FLASH_WR 51
#define ERASE_SEC 52
#define FLASHDIR 0x00001F00
// Call into embedded utility.
#define IAP_LOCATION 0x1fff1ff1
void (*iap_entry)(unsigned long *, unsigned long *) = (void *)IAP_LOCATION;
unsigned long command[5];
unsigned long result[4];
// FLASHDIR is the location in flash to write.
// sys_state is the structure in RAM to write to flash.
void
write_state(void){
// First we will prepare the sector for erase.
command[0] = PREP_WR;
command[1] = FLASHDIR >> 12;
command[2] = FLASHDIR >> 12;
iap_entry (command, result);
if (result[0] != 0) {
// This is an error.
result[0] = 0xff;
return;
}
// Next we will erase the sector.
command[0] = ERASE_SEC;
command[1] = FLASHDIR;
command[2] = FLASHDIR;
command[3] = SystemFrequency / 1000; // Must be in terms of cycles per millisecond.
command[4] = 0;
iap_entry (command, result);
if (result[0] != 0) {
// This is an error.
result[0] = 0xff;
return (-1);
}
// We will prepare the sector for write.
command[0] = PREP_WR;
command[1] = FLASHDIR >> 12;
command[2] = FLASHDIR >> 12;
iap_entry (command, result);
if (result[0] != 0) {
// This is an error.
result[0] = 0xff;
return;
}
// Then we will write the sector.
command[0] = FLASH_WR;
command[1] = FLASHDIR;
command[2] = vRef; //extern volatile uint32_t vRef = 3300;
command[3] = 256;
command[4] = SystemFrequency / 1000; // Must be in terms of cycles per millisecond.
iap_entry (command, result);
if (result[0] != 0) {
// This is an error.
result[0] = 0xff;
return;
}
}


uint32_t read_nonvolatile_parameters(void)
{

unsigned long *add;
uint32_t value=0;

add=(unsigned long *)FLASHDIR;
value=*add;
}
*/


void init() {
	uint8_t version[2];
//NEED		eeprom_read_block(version,(const uint8_t*)eeprom::VERSION_LOW,2);
	if ((version[1]*100+version[0]) == firmware_version) return;
	if (version[1] == 0xff || version[1] < 2) {
		// Initialize eeprom map
		// Default: enstops inverted, Y axis inverted
		uint8_t axis_invert = 1<<1; // Y axis = 1
		uint8_t endstop_invert = 0b00010111; // all endstops inverted
//NEED			eeprom_write_byte((uint8_t*)eeprom::AXIS_INVERSION,axis_invert);
//NEED			eeprom_write_byte((uint8_t*)eeprom::ENDSTOP_INVERSION,endstop_invert);
//NEED			eeprom_write_byte((uint8_t*)eeprom::MACHINE_NAME,0); // name is null
	}
	// Write version
	version[0] = firmware_version % 100;
	version[1] = firmware_version / 100;
//NEED		eeprom_write_block(version,(uint8_t*)eeprom::VERSION_LOW,2);
}

uint8_t getEeprom8(const uint16_t location, const uint8_t default_value) {
	uint8_t data;
//NEED		eeprom_read_block(&data,(const uint8_t*)location,1);
	if (data == 0xff) data = default_value;
	return data;
}

uint16_t getEeprom16(const uint16_t location, const uint16_t default_value) {
	uint16_t data;
//NEED		eeprom_read_block(&data,(const uint8_t*)location,2);
	if (data == 0xffff) data = default_value;
	return data;
}

} // namespace eeprom
