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

#include "UART.hh"
#include <stdint.h>
extern "C" {
	#include "usbhw.h"
	#include "usbcfg.h"
	#include "cdcuser.h"
	#include "lpc17xx_uart.h"
	#include "lpc17xx_pinsel.h"
}
//#include <avr/sfr_defs.h>
//#include <avr/interrupt.h>
//#include <avr/io.h>
//#include <util/delay.h>
#include "Delay_ms.hh"
#include "Configuration.hh"

#define FIFO_Enabled 0
#define LPC_UART_NO LPC_UART1

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

/*
// MEGA644P_DOUBLE_SPEED_MODE is 1 if USXn is 1.
#ifndef MEGA644P_DOUBLE_SPEED_MODE
#define MEGA644P_DOUBLE_SPEED_MODE 1
#endif

#if MEGA644P_DOUBLE_SPEED_MODE
#define UBRR0_VALUE 16  // 115200 baud
#define UBRR1_VALUE 51  // 38400 baud
#define UCSRA_VALUE(uart_) _BV(U2X##uart_)
#else
#define UBRR0_VALUE 8   // 115200
#define UBRR1_VALUE 25  // 38400 baud
#define UCSRA_VALUE(uart_) 0
#endif

// Adapted from ancient arduino/wiring rabbit hole
#define INIT_SERIAL(uart_) \
{ \
    UBRR##uart_##H = UBRR##uart_##_VALUE >> 8; \
    UBRR##uart_##L = UBRR##uart_##_VALUE & 0xff; \
    \
    // set config for uart_
    UCSR##uart_##A = UCSRA_VALUE(uart_); \
    UCSR##uart_##B = _BV(RXEN##uart_) | _BV(TXEN##uart_); \
    UCSR##uart_##C = _BV(UCSZ##uart_##1)|_BV(UCSZ##uart_##0); \
    // defaults to 8-bit, no parity, 1 stop bit  \
}



#define ENABLE_SERIAL_INTERRUPTS(uart_) \
{ \
	UCSR##uart_##B |=  _BV(RXCIE##uart_) | _BV(TXCIE##uart_); \
}

#define DISABLE_SERIAL_INTERRUPTS(uart_) \
{ \
	UCSR##uart_##B &= ~(_BV(RXCIE##uart_) | _BV(TXCIE##uart_)); \
}
*/

UART UART::uart[2] = {
		UART(0),
		UART(1)
};

volatile uint8_t loopback_bytes = 0;

// Unlike the old implementation, we go half-duplex: we don't listen while sending.
inline void listen() {
	TX_ENABLE_PIN.setValue(false);
}

inline void speak() {
	TX_ENABLE_PIN.setValue(true);
}

UART::UART(uint8_t index) : index_(index), enabled_(false) {
	if (index_ == 0) {
		// Init USB for UART
		USB_Init();						// USB Initialization
		while (!USB_Configuration);		// wait until USB is configured
	} else if (index_ == 1) {
		// UART Configuration Structure
		UART_CFG_Type u_cfg;
		u_cfg.Baud_rate = 38400;
		u_cfg.Databits = UART_DATABIT_8;
		u_cfg.Parity = UART_PARITY_NONE;
		u_cfg.Stopbits = UART_STOPBIT_1;
		UART_Init((LPC_UART_TypeDef *)LPC_UART_NO, &u_cfg);
		// Initialize UART0 pin connect
		PINSEL_CFG_Type PinCfg;
		if (LPC_UART_NO == LPC_UART0){
			PinCfg.Funcnum = 1;
			PinCfg.OpenDrain = 0;
			PinCfg.Pinmode = 0;
			PinCfg.Pinnum = 2;
			PinCfg.Portnum = 0;
			PINSEL_ConfigPin(&PinCfg);
			PinCfg.Pinnum = 3;
			PINSEL_ConfigPin(&PinCfg);
		} else if (LPC_UART_NO == LPC_UART1){
			PinCfg.Funcnum = 1;
			PinCfg.OpenDrain = 0;
			PinCfg.Pinmode = 0;
			PinCfg.Pinnum = 15;
			PinCfg.Portnum = 0;
			PINSEL_ConfigPin(&PinCfg);
			PinCfg.Pinnum = 16;
			PINSEL_ConfigPin(&PinCfg);
		}
#ifndef FIFO_Enabled
		// UART FIFO Configuration Structure
		UART_FIFO_CFG_Type fifo_cfg;
		fifo_cfg.FIFO_ResetRxBuf = ENABLE;
		fifo_cfg.FIFO_ResetTxBuf = ENABLE;
		fifo_cfg.FIFO_DMAMode = DISABLE;
		fifo_cfg.FIFO_Level = UART_FIFO_TRGLEV0;
		UART_FIFOConfig((LPC_UART_TypeDef *)LPC_UART_NO, &fifo_cfg);
#endif
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
	uint8_t send_byte = out.getNextByteToSend();
	if (index_ == 0) {		//uart0 eg usb
		// get first 2 bytes, second = size
#ifndef FIFO_Enabled
		char serBuf[USB_CDC_BUFSIZE];
		serBuf[0] = send_byte;
		serBuf[1] = out.getNextByteToSend();
		// remaining bytes
		for (uint8_t i = 2; i > (serBuf[1] + 3); i++){
			serBuf[i] = out.getNextByteToSend();
		}
		// send all bytes
		USB_WriteEP (CDC_DEP_IN, (unsigned char *)&serBuf[0], serBuf[1]);
#else
		USB_WriteEP (CDC_DEP_IN, &send_byte, 1);
#endif
	} else if (index_ == 1) {
		speak();
//		_delay_us(10);
		Delay (1);						//NEED to reduce delay from 1ms to 10us
		loopback_bytes = 1;
		UART_SendByte((LPC_UART_TypeDef *)LPC_UART_NO, send_byte);  // NEED to choose which UART
	}
}

void UART::enable(bool enabled) {
	enabled_ = enabled;
	if (index_ == 0) {
		if (enabled) {
			USB_Connect(TRUE);			// USB Connect
		}
		else {
			USB_Connect(FALSE);			// USB Disconnect
		}
	} else if (index_ == 1) {
		if (enabled) { UART_TxCmd((LPC_UART_TypeDef *)LPC_UART_NO, ENABLE); }
		else { UART_TxCmd((LPC_UART_TypeDef *)LPC_UART_NO, DISABLE); }
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

// Send and receive interrupts
//ISR(USART0_RX_vect)
//{
//	UART::uart[0].in.processByte( UDR0 );
//}

volatile uint8_t byte_in;

//ISR(USART1_RX_vect)
void UART1_IRQHandler(void)
{
	uint32_t intsrc, tmp, tmp1;
	/* Determine the interrupt source */
	intsrc = UART_GetIntId((LPC_UART_TypeDef *)LPC_UART_NO);
	tmp = intsrc & UART_IIR_INTID_MASK;
	// Receive Line Status
	if (tmp == UART_IIR_INTID_RLS){
		// Check line status
		tmp1 = UART_GetLineStatus(LPC_UART_NO);
		// Mask out the Receive Ready and Transmit Holding empty status
		tmp1 &= (UART_LSR_OE | UART_LSR_PE | UART_LSR_FE | UART_LSR_BI | UART_LSR_RXFE);
		// If any error exist
		// if (tmp1) {
		//	UART_IntErr(tmp1);
		// }
	}
	// Receive Data Available or Character time-out
	if ((tmp == UART_IIR_INTID_RDA) || (tmp == UART_IIR_INTID_CTI)) {
		UART::uart[1].in.processByte(UART_ReceiveByte((LPC_UART_TypeDef *)LPC_UART_NO));
	}

	// Transmit Holding Empty
	if (tmp == UART_IIR_INTID_THRE){
		if (UART::uart[1].out.isSending()) {
			loopback_bytes++;
			UART_SendByte((LPC_UART_TypeDef *)LPC_UART_NO, UART::uart[1].out.getNextByteToSend());  // NEED to choose which UART
		} else {
			//	_delay_us(10);
				Delay (1);						//NEED to reduce delay from 1ms to 10us
			listen();
		}
	}
}
