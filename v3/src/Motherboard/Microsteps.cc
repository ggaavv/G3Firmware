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



#include "Microsteps.hh"
#include "lpc17xx_i2c.h"
#include "LPC17xx.h"
#include "EepromMap.hh"

namespace microsteps {

void init() {

	/* Init I2C pin connect
	 Connecterd to
	 SDA1 - P0.19
	 SCL1 - P0.20
	 */
	PINSEL_CFG_Type PinCfg;
	I2C_M_SETUP_Type transferMCfg;

	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Funcnum = 3;
	PinCfg.Portnum = 0;
	PinCfg.Pinnum = 19;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 20;
	PINSEL_ConfigPin(&PinCfg);

	I2C_Init(LPC_I2C1, 200000);
	I2C_Cmd(LPC_I2C1, ENABLE);;

	/* Create packet buffer */
	uint8_t Master_Buf[BUFFER_SIZE];

	/* Configure I2C slave device first */
	transferMCfg.sl_addr7bit = I2CDEV_S_ADDR;
	transferMCfg.tx_data = Master_Buf;
	transferMCfg.tx_length = sizeof(Master_Buf);
	transferMCfg.rx_data = 0;
	transferMCfg.rx_length = 0;
	transferMCfg.retransmissions_max = 3;

	/* Fill buffer with I2C config settings*/
	/* Slave address | Command | Data to Register | Data to Register */
	Master_Buf[0] = 0x06;  // Command - Configuration port 0
	Master_Buf[1] = 0x00;  // Data to Register - all port0 Outputs
	Master_Buf[2] = 0x00;  // Data to Register - all port1 Outputs
	/* Send config to I2C */
	I2C_MasterTransferData(LPC_I2C1, &transferMCfg, I2C_TRANSFER_POLLING);

	/* Fill buffer with I2C port outputs settings*/
	/* Slave address | Command | Port0 Data | Port1 Data */
	Master_Buf[0] = 0x02;  // Command - Output port 0
	Master_Buf[1] = *(uint32_t*)eeprom::MICROSTEPS_P0;  // Data to Register - all port0 Outputs
	Master_Buf[2] = *(uint32_t*)eeprom::MICROSTEPS_P1;  // Data to Register - all port1 Outputs
	/* Send port output to I2C */
	I2C_MasterTransferData(LPC_I2C1, &transferMCfg, I2C_TRANSFER_POLLING);
}

}
