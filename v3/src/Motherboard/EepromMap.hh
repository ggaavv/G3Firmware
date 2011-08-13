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


#ifndef EEPROMMAP_HH_
#define EEPROMMAP_HH_

#include <stdint.h>

void read_all_from_flash (void);
void save_to_flash (void);

namespace eeprom {

// uint32_t EEPROM_START_ADDRESS		= (uint32_t*)0x10001000;
#define EEPROM_START_ADDRESS			(0x10001000UL)

//uint32_t EEPROM_SIZE					= 0x00001000;
#define EEPROM_SIZE         			(0x00001000UL)
/// Version, low byte: 1 byte
//uint32_t *VERSION_LOW					= (uint32_t*)0x10000000;
#define VERSION_LOW						(EEPROM_START_ADDRESS + 0x00000)
/// Version, high byte: 1 byte
//uint32_t *VERSION_HIGH				= (uint32_t*)0x10000004;
#define VERSION_HIGH					(EEPROM_START_ADDRESS + 0x00004)

// Axis inversion flags: 1 byte.
// Axis N (where X=0, Y=1, etc.) is inverted if the Nth bit is set.
//uint32_t *AXIS_INVERSION				= (uint32_t*)0x10000008;
#define AXIS_INVERSION					(EEPROM_START_ADDRESS + 0x00008)

// Endstop inversion flags: 1 byte.
// The endstops for axis N (where X=0, Y=1, etc.) are considered
// to be logically inverted if the Nth bit is set.
// Bit 7 is set to indicate endstops are present; it is zero to indicate
// that endstops are not present.
// Ordinary endstops (H21LOB et. al.) are inverted.
//uint32_t *ENDSTOP_INVERSION			= (uint32_t*)0x1000000c;
#define ENDSTOP_INVERSION				(EEPROM_START_ADDRESS + 0x0000c)

// Name of this machine: 32 bytes.
//uint32_t *MACHINE_NAME				= (uint32_t*)0x10000040;
#define MACHINE_NAME					(EEPROM_START_ADDRESS + 0x00040)

// Default locations for the axis: 5 x 32 bit = 20 bytes
//uint32_t *AXIS_HOME_POSITIONS			= (uint32_t*)0x10000800;
#define AXIS_HOME_POSITIONS				(EEPROM_START_ADDRESS + 0x00800)

void init();

uint8_t getEeprom8(uint32_t *location, const uint8_t default_value);
uint16_t getEeprom16(uint32_t *location, const uint16_t default_value);

} // namespace eeprom

#endif // EEPROMMAP_HH_
