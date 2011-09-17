/*
 * Copyright 2010 by Gavin Clarke
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


#ifndef MICROSTEPS_HH_
#define MICROSTEPS_HH_

#include <stdint.h>

namespace microsteps {
uint32_t I2CDEV_S_ADDR = 0x20;

#define BUFFER_SIZE	0x2
uint8_t Port0_I2C = 0;
uint8_t Port1_I2C = 0;

void init();

}

#endif // MICROSTEPS_HH_
