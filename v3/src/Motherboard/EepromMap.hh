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

void erase_program_variables (void);

namespace eeprom {

uint32_t *EEPROM_SIZE_R				= (uint32_t*)0x00001000;
uint32_t *EEPROM_SIZE_W				= (uint32_t*)0x10001000;
/// Version, low byte: 1 byte
uint32_t *VERSION_LOW_R				= (uint32_t*)0x00000000;
uint32_t *VERSION_LOW_W				= (uint32_t*)0x10000000;
/// Version, high byte: 1 byte
uint32_t *VERSION_HIGH_R				= (uint32_t*)0x00000004;
uint32_t *VERSION_HIGH_W				= (uint32_t*)0x10000004;

// Axis inversion flags: 1 byte.
// Axis N (where X=0, Y=1, etc.) is inverted if the Nth bit is set.
uint32_t *AXIS_INVERSION_R			= (uint32_t*)0x00000008;
uint32_t *AXIS_INVERSION_W			= (uint32_t*)0x10000008;

// Endstop inversion flags: 1 byte.
// The endstops for axis N (where X=0, Y=1, etc.) are considered
// to be logically inverted if the Nth bit is set.
// Bit 7 is set to indicate endstops are present; it is zero to indicate
// that endstops are not present.
// Ordinary endstops (H21LOB et. al.) are inverted.
uint32_t *ENDSTOP_INVERSION_R			= (uint32_t*)0x0000000c;
uint32_t *ENDSTOP_INVERSION_W			= (uint32_t*)0x1000000c;

// Name of this machine: 32 bytes.
uint32_t *MACHINE_NAME_R				= (uint32_t*)0x00000040;
uint32_t *MACHINE_NAME_W				= (uint32_t*)0x10000040;

// Default locations for the axis: 5 x 32 bit = 20 bytes
uint32_t *AXIS_HOME_POSITIONS_R		= (uint32_t*)0x00000800;
uint32_t *AXIS_HOME_POSITIONS_W		= (uint32_t*)0x10000800;

void init();

uint8_t getEeprom8(uint32_t *location, const uint8_t default_value);
uint16_t getEeprom16(uint32_t *location, const uint16_t default_value);

} // namespace eeprom

#endif // EEPROMMAP_HH_
