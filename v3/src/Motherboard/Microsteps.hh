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
#include "lpc17xx_pinsel.h"
#include "LPC17xx.h"
#include "Configuration.hh"

namespace microsteps {
uint32_t I2CDEV_S_ADDR = 0x20;
uint32_t pin_port_map;

#define BUFFER_SIZE	0x2
uint8_t Port0_I2C = 0;
uint8_t Port1_I2C = 0;

enum step_dev {
	Full_step,
	Half_step,
	Quarter_step,
	Eighth_step,
	Sixteenth_step
};

// Microstepping
	/*	MS1 	MS2 	MS3 	Microstep Resolution
		Low 	Low 	Low 	Full step
		High 	Low 	Low 	Half step
		Low 	High 	Low 	Quarter step
		High 	High 	Low 	Eighth step
		High 	High 	High 	Sixteenth step	*/

bool X_MS1 = 1;
bool X_MS2 = 1;
bool X_MS3 = 1;
bool Y_MS1 = 1;
bool Y_MS2 = 1;
bool Y_MS3 = 1;
bool Z_MS1 = 1;
bool Z_MS2 = 1;
bool Z_MS3 = 1;
bool A_MS1 = 1;
bool A_MS2 = 1;
bool A_MS3 = 1;
bool B_MS1 = 1;
bool B_MS2 = 1;
bool B_MS3 = 1;

//				[STEPPER_COUNT][MSx][port, no, MSx]
uint8_t microstep_port_array [5][3][3] = {
			//	port no MS1		port no MS2		port no MS3
	/* X */	{	{0,  7, X_MS1},	{0,  6, X_MS2},	{0,  5, X_MS3}	},	/* X */
	/* Y */	{	{0,  3, Y_MS1},	{0,  2, Y_MS2},	{0,  1, Y_MS3}	},	/* Y */
	/* Z */	{	{0,  1, Z_MS1},	{1,  0, Z_MS2},	{1,  7, Z_MS3}	},	/* Z */
	/* A */	{	{1,  6, A_MS1},	{1,  5, A_MS2},	{1,  4, A_MS3}	},	/* A */
	/* B */	{	{1,  3, B_MS1},	{1,  2, B_MS2},	{1,  1, B_MS3}	}	/* B */
};

void init();
uint8_t microstep_pinout(bool port_no);
}

#endif // MICROSTEPS_HH_
