/*
 * Copyright 2010 by Adam Mayer	 <adam@makerbot.com>
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

//#include "usbhw.hh"
//#include "usbcfg.hh"
//#include "cdcuser.hh"
//#include "usbcore.hh"
#include "UART.hh"
#include <stdint.h>

extern "C" {
	#include "lpc17xx_uart.h"
	#include "lpc17xx_pinsel.h"
	#include "LPC17xx.h"
//	#include "usbSerial.h"
//	#include "usb.h"
//	#include "serial.h"

	#include "usbhw.h"
	#include "usbcfg.h"
	#include "cdcuser.h"
	#include "usbcore.h"
}
//#include <avr/sfr_defs.h>
//#include <avr/interrupt.h>
//#include <avr/io.h>
//#include <util/delay.h>
#include "Delay.hh"

/********************************/
#include "test.hh"  // testing
#include "test_led.hh"  // testing
#include "test_u.hh"
#include "Uart32.c"
//#include "Delay.hh"
//	#include "lpc17xx_nvic.h"
//	#include "lpc17xx_timer.h"
//	#include "LPC17xx.h"
//test_led(1);
/********************************/

//#define FIFO_Enabled 1

// TODO: There should be a better way to enable this flag?
#if ASSERT_LINE_FIX
#include "ExtruderBoard.hh"
#endif

// We have to track the number of bytes that have been sent, so that we can filter
// them from our receive buffer later.This is only used for RS485 mode.
volatile uint8_t loopback_bytes = 0;

/*
NOTE: you will need to call SystemCoreClockUpdate() as the very
first line in your main function. This will update the various
registers and constants to allow accurate timing.
If you include the following files in you compilation and include
the call SerialInit(UART0,9600); in your main function, you
*should* be able to use uart_putchar(char,NULL) to output
individual characters to the USB UART, and use putst(const char * s);
to output a whole string.
*/

UART UART::uart[2] = {
		UART(0),
		UART(1)
};

// Transition to a non-transmitting state. This is only used for RS485 mode.
inline void listen() {
	TX_ENABLE_PIN.setValue(false);
}

// Transition to a transmitting state
inline void speak() {
	TX_ENABLE_PIN.setValue(true);
}

UART::UART(uint8_t index) : index_(index), enabled_(false) {
	if (index_ == 0) {
		// Init USB for UART
//		usbSerialInit();
//		int c = 21;
//		VCOM_putchar(c);
//		VCOM_Init();					// VCOM Initialization
//		USB_Init();						// USB Initialization
//		USB_Connect(TRUE);
			uint8_t menu322[] = "b4 USB config\n";
			UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu322, sizeof(menu322), BLOCKING);
		while (!USB_Configuration);		// wait until USB is configured
			uint8_t menu332[] = "after USB config\n";
			UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu332, sizeof(menu332), BLOCKING);
	} else if (index_ == 1) {
		// UART Configuration Structure
		UART_CFG_Type u_cfg;
		u_cfg.Baud_rate = 38400;
		u_cfg.Databits = UART_DATABIT_8;
		u_cfg.Parity = UART_PARITY_NONE;
		u_cfg.Stopbits = UART_STOPBIT_1;
		UART_Init((LPC_UART_TypeDef *)LPC_UART1, &u_cfg);
		// Initialize UART0 pin connect
		PINSEL_CFG_Type PinCfg;
		PinCfg.Funcnum = 1;
		PinCfg.OpenDrain = 0;
		PinCfg.Pinmode = 0;
		PinCfg.Pinnum = 0;
		PinCfg.Portnum = 0;
		PINSEL_ConfigPin(&PinCfg);
		PinCfg.Pinnum = 1;
		PINSEL_ConfigPin(&PinCfg);
		NVIC_SetPriority(UART1_IRQn, 8);
		NVIC_EnableIRQ(UART1_IRQn);
		// UART1 is an RS485 port, and requires additional setup.
		// Read enable: PD5, active low
		// Tx enable: PD4, active high
		TX_ENABLE_PIN.setDirection(true);
		RX_ENABLE_PIN.setDirection(true);
		RX_ENABLE_PIN.setValue(false);  // Active low
		listen();
	}
}

/// UART bytes will be triggered by the tx complete interrupt.
/// USB bytes sent as whole packets
void UART::beginSend() {
	if (!enabled_) { return; }
	if (index_ == 0) {		//uart0 eg usb
		char serBuf[USB_CDC_BUFSIZE];
		while (UART::uart[0].out.isSending()) {
			uint32_t i;
			for (i = 0; i > USB_CDC_BUFSIZE; i++){
				serBuf[i] = out.getNextByteToSend();
				if (!(UART::uart[0].out.isSending()))
				break;
			}
			USB_WriteEP (CDC_DEP_IN, (unsigned char *)&serBuf[0], i);
		}
	} else if (index_ == 1) {
		speak();
		_delay_us(10);
		loopback_bytes = 1;
		UART_SendByte((LPC_UART_TypeDef *)LPC_UART1, out.getNextByteToSend());
	}
}

void UART::enable(bool enabled) {
	enabled_ = enabled;
	if (index_ == 0) {
		if (enabled) {
			USB_Connect(TRUE);      // USB Connect
//			USBHwConnect(TRUE);			// USB Connect
		}
		else {
			USB_Connect(FALSE);      // USB Disconnect
//			USBHwConnect(FALSE);			// USB Disconnect
		}
	} else if (index_ == 1) {
		if (enabled){
			UART_TxCmd((LPC_UART_TypeDef *)LPC_UART1, ENABLE);
		}
		else {
			UART_TxCmd((LPC_UART_TypeDef *)LPC_UART1, DISABLE);
		}
	}
}

// Reset the UART to a listening state.  This is important for
// RS485-based comms.
void UART::reset() {
	if (index_ == 1) {
		loopback_bytes = 0;
		listen();
	}
}

volatile uint8_t byte_in;

extern "C" void UART1_IRQHandler(void){
	uint8_t menu910[] = "\n UIRQ1 ";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu910, sizeof(menu910), BLOCKING);

	uint32_t intsrc, tmp, tmp1;
	// Determine the interrupt source
	intsrc = UART_GetIntId((LPC_UART_TypeDef *)LPC_UART1);
	tmp = intsrc & UART_IIR_INTID_MASK;
	// Receive Line Status
	if (tmp == UART_IIR_INTID_RLS){
		// Check line status
		tmp1 = UART_GetLineStatus((LPC_UART_TypeDef *)LPC_UART1);
		// Mask out the Receive Ready and Transmit Holding empty status
		tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_BI | UART_LSR_RXFE);
		// If any error exist
		// if (tmp1) {
		//	UART_IntErr(tmp1);
		// }
	}
	// Receive Data Available or Character time-out
	if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI)) {
		static uint8_t byte_in;
		byte_in = UART_ReceiveByte((LPC_UART_TypeDef *)LPC_UART1);
		if (loopback_bytes > 0) {
			loopback_bytes--;
		} else {
			UART::getSlaveUART().in.processByte( byte_in );
		}
	}

	// Transmit Holding Empty
	if (tmp == UART_IIR_INTID_THRE){
		if (UART::getSlaveUART().out.isSending()) {
			loopback_bytes++;
			UART_SendByte((LPC_UART_TypeDef *)LPC_UART1, UART::getSlaveUART().out.getNextByteToSend());  // NEED to choose which UART
		} else {
			_delay_us(10);
			listen();
		}
	}
}

unsigned char BulkBufOut  [USB_CDC_BUFSIZE];

extern "C" void CANActivity_IRQHandler(void){
	uint8_t menu910[] = "\nCQ";
	UART_Send((LPC_UART_TypeDef *)LPC_UART2, menu910, sizeof(menu910), BLOCKING);
	int numBytesRead = USB_ReadEP(CDC_DEP_OUT, &BulkBufOut[0]);
	for (int i = 0; i < numBytesRead; i++){
		UART_8((LPC_UART_TypeDef *)LPC_UART2, BulkBufOut[i]);
		UART::uart[0].in.processByte( BulkBufOut[i] );
	}
}


//  int numBytesRead;

  // get data from USB into intermediate buffer
//  numBytesRead = USB_ReadEP(CDC_DEP_OUT, &BulkBufOut[0]);

  // ... add code to check for overwrite

  // store data in a buffer to transmit it over serial interface
//  CDC_WrOutBuf ((char *)&BulkBufOut[0], &numBytesRead);
