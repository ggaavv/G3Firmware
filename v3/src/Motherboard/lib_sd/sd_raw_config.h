
/*
 * Copyright (c) 2006-2010 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef SD_RAW_CONFIG_H
#define SD_RAW_CONFIG_H

#include <stdint.h>
#include "lpc17xx_gpio.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \addtogroup sd_raw
 *
 * @{
 */
/**
 * \file
 * MMC/SD support configuration (license: GPLv2 or LGPLv2.1)
 */

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD write support.
 *
 * Set to 1 to enable MMC/SD write support, set to 0 to disable it.
 */
#define SD_RAW_WRITE_SUPPORT 1

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD write buffering.
 *
 * Set to 1 to buffer write accesses, set to 0 to disable it.
 *
 * \note This option has no effect when SD_RAW_WRITE_SUPPORT is 0.
 */
#define SD_RAW_WRITE_BUFFERING 1

/**
 * \ingroup sd_raw_config
 * Controls MMC/SD access buffering.
 * 
 * Set to 1 to save static RAM, but be aware that you will
 * lose performance.
 *
 * \note When SD_RAW_WRITE_SUPPORT is 1, SD_RAW_SAVE_RAM will
 *       be reset to 0.
 */
#define SD_RAW_SAVE_RAM 1

/**
 * \ingroup sd_raw_config
 * Controls support for SDHC cards.
 *
 * Set to 1 to support so-called SDHC memory cards, i.e. SD
 * cards with more than 2 gigabytes of memory.
 */
#define SD_RAW_SDHC 0

/**c
 * @}
 */

/* defines for customisation of sd/mmc port access */
#if defined(__LPC17xx_H__)

//GPIO_SetDir(uint8_t portNum, uint32_t bitValue, uint8_t dir)
//GPIO_SetValue(uint8_t portNum, uint32_t bitValue)
//GPIO_ReadValue(uint8_t portNum)

/* Define as 1 if and SD card slot is present; 0 if not.
#define HAS_SD          1
// The pin that connects to the write protect line on the SD header.
#define SD_WRITE_PIN    Pin(Port1,0)
// The pin that connects to the card detect line on the SD header.
#define SD_DETECT_PIN   Pin(Port1,1)
// The pin that connects to the chip select line on the SD header.
#define SD_SELECT_PIN   Pin(Port1,0)
*/

// The pin that connects to the write protect line on the SD header.
#define SD_WRITE_PIN_Port    1
#define SD_WRITE_PIN_Pin    (0<<1)

// The pin that connects to the card detect line on the SD header.
#define SD_DETECT_PIN_Port   1
#define SD_DETECT_PIN_Pin   (0<<1)

// The pin that connects to the chip select line on the SD header.
#define SD_SELECT_PIN_Port   1
#define SD_SELECT_PIN_Pin   (0<<1)

#define MOSI_PIN_Port	1
#define MOSI_PIN_Pin	(0<<1)
#define SCK_PIN_Port   1
#define SCK_PIN_Pin   (0<<4)
#define SS_PIN_Port   1
#define SS_PIN_Pin   (0<<1)
#define MISO_PIN_Port   1
#define MISO_PIN_Pin   (0<<1)

#define configure_pin_mosi() GPIO_SetDir(MOSI_PIN_Port, MOSI_PIN_Pin, 1)
#define configure_pin_sck() GPIO_SetDir(SCK_PIN_Port, SCK_PIN_Pin, 1)
#define configure_pin_ss() GPIO_SetDir(SS_PIN_Port, SS_PIN_Pin, 1)
#define configure_pin_miso() GPIO_SetDir(MISO_PIN_Port, MISO_PIN_Pin, 1)

//#define select_card() (SD_SELECT_PIN.setDirection(true), SD_SELECT_PIN.setValue(true));
#define select_card() (GPIO_SetDir(SD_SELECT_PIN_Port, SD_SELECT_PIN_Pin, 1), GPIO_SetValue(SD_SELECT_PIN_Port, ((GPIO_ReadValue(SD_SELECT_PIN_Port)&SD_SELECT_PIN_Port) & SD_SELECT_PIN_Pin) | (1?SD_SELECT_PIN_Pin:0)));
//#define unselect_card() (SD_SELECT_PIN.setDirection(false), SD_SELECT_PIN.setValue(false));
#define unselect_card() (GPIO_SetDir(SD_SELECT_PIN_Port, SD_SELECT_PIN_Pin, 1), GPIO_SetValue(SD_SELECT_PIN_Port, ((GPIO_ReadValue(SD_SELECT_PIN_Port)&SD_SELECT_PIN_Port) & SD_SELECT_PIN_Pin) | (0?SD_SELECT_PIN_Pin:0)));


#else
    #error "no sd/mmc pin mapping available!"
#endif

//#define configure_pin_available() SD_DETECT_PIN.setDirection(false)
#define configure_pin_available() GPIO_SetDir(SD_DETECT_PIN_Port, SD_DETECT_PIN_Pin, 0)
//#define configure_pin_locked() SD_WRITE_PIN.setDirection(false)
#define configure_pin_locked() GPIO_SetDir(SD_WRITE_PIN_Port, SD_WRITE_PIN_Pin, 0)

//#define get_pin_available() SD_DETECT_PIN.getValue()
#define get_pin_available() ((GPIO_ReadValue(SD_DETECT_PIN_Port)&SD_DETECT_PIN_Pin) != 0)
//#define get_pin_locked() !SD_WRITE_PIN.getValue()
#define get_pin_locked() !((GPIO_ReadValue(SD_WRITE_PIN_Port)&SD_WRITE_PIN_Pin) != 0)

#if SD_RAW_SDHC
    typedef uint64_t offset_t;
#else
    typedef uint32_t offset_t;
#endif

/* configuration checks */
#if SD_RAW_WRITE_SUPPORT
#undef SD_RAW_SAVE_RAM
#define SD_RAW_SAVE_RAM 0
#else
#undef SD_RAW_WRITE_BUFFERING
#define SD_RAW_WRITE_BUFFERING 0
#endif

#ifdef __cplusplus
}
#endif

#endif

