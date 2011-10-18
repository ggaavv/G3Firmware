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
uint8_t microstep_pinout(uint8_t port_no);

#define eeprom_address(x)   (*(volatile unsigned long *)(x))

#define EEPROM_FLASH_AREA_START		(0x00002000)

#define EEPROM_START_ADDRESS		(0x10007000)

#define EEPROM_SIZE         		(0x00001000)
/// Version, low byte: 1 byte
#define VERSION_LOW					(EEPROM_START_ADDRESS + 0x0000)
/// Version, high byte: 1 byte
#define VERSION_HIGH				(EEPROM_START_ADDRESS + 0x0001)

// Axis inversion flags: 1 byte.
// Axis N (where X=0, Y=1, etc.) is inverted if the Nth bit is set.
#define AXIS_INVERSION				(EEPROM_START_ADDRESS + 0x0002)

// Endstop inversion flags: 1 byte.
// The endstops for axis N (where X=0, Y=1, etc.) are considered
// to be logically inverted if the Nth bit is set.
// Bit 7 is set to indicate endstops are present; it is zero to indicate
// that endstops are not present.
// Ordinary endstops (H21LOB et. al.) are inverted.
#define ENDSTOP_INVERSION			(EEPROM_START_ADDRESS + 0x0003)

// Name of this machine: 32 bytes.
#define MACHINE_NAME				(EEPROM_START_ADDRESS + 0x0020)

// Default locations for the axis: 5 x 32 bit = 20 bytes
#define AXIS_HOME_POSITIONS			(EEPROM_START_ADDRESS + 0x0060)
;
#define MICROSTEPS_P0				(EEPROM_START_ADDRESS + 0x0100)
#define MICROSTEPS_P1				(EEPROM_START_ADDRESS + 0x0110)

enum step_dev {
	Full_step,
	Half_step,
	Quarter_step,
	Eighth_step,
	Sixteenth_step
};

namespace eeprom {

void init();

uint8_t getEeprom8(uint32_t location, const uint8_t default_value);
uint16_t getEeprom16(uint32_t location, const uint16_t default_value);

} // namespace eeprom

#endif // EEPROMMAP_HH_
