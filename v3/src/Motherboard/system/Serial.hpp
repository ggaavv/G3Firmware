/*
 * Serial.h
 *
 *  Created on: Dec 4, 2010
 *      Author: dejagerd
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include <LPC17xx.h>
#include <stdio.h>
#include <errno.h>
#include "newlibMinimal.h"

/* dtab for an example stream device called "com1" */
//const devoptab_t devoptab_tty0 = {
//		"tty0",
//		NULL,//com1_open_r,
//		NULL,//com1_close_r,
//		NULL,//com1_write_r,
//		NULL//com1_read_r
//};
///Enumeration listing port configurations.
typedef enum {
	///Primary configuration of uart0
	///Transmitting on p0.2
	///Receiving on p0.3
	UART0 = 0,
	///Primary configuration of uart1
	///Transmitting on p0.15
	///Receiving on p0.16
	UART1 = 1,
	///Primary configuration of uart2
	///Transmitting on p0.10
	///Receiving on p0.11
	UART2 = 2,
	///Primary configuration of uart3
	///Transmitting on p0.0
	///Receiving on p0.1
	UART3 = 3,
	///Secondary configuration of uart1
	///Transmitting on p2.0
	///Receiving on p2.1
	UART1_ALT1 = 5,
	///Secondary configuration of uart2
	///Transmitting on p2.8
	///Receiving on p2.9
	UART2_ALT1 = 6,
	///Secondary configuration of uart3
	///Transmitting on p0.25
	///Receiving on p0.26
	UART3_ALT1 = 7,
	///Tertiary configuration of uart3
	///Transmitting on p4.28
	///Receiving on p4.29
	UART3_ALT2 = 11
} UART_PortConfig;

/**
 * Changes the configuration of the current port.
 * @param config Chooses the configuration for this port
 * @param baudrate Baudrate to use on this port
 * @return True if successful, FALSE if failed
 */
int SerialInit(UART_PortConfig config, uint32_t baudrate) {
	uint32_t Fdiv;
	uint32_t pclkdiv, pclk;
	uint32_t UART;
	switch (config % 4) {
	case 0:
		UART = (uint32_t) LPC_UART0;
		break;
	case 1:
		UART = (uint32_t) LPC_UART1;
		break;
	case 2:
		UART = (uint32_t) LPC_UART2;
		break;
	case 3:
		UART = (uint32_t) LPC_UART3;
		break;
	default:
		return 0;
	}

	pclkdiv = (LPC_SC->PCLKSEL0 >> 6) & 0x03;
	switch (pclkdiv) {
	case 0x00:
	default:
		pclk = SystemCoreClock / 4;
		break;
	case 0x01:
		pclk = SystemCoreClock;
		break;
	case 0x02:
		pclk = SystemCoreClock / 2;
		break;
	case 0x03:
		pclk = SystemCoreClock / 8;
	}
	Fdiv = (pclk / 16) / baudrate; /*baud rate */
	switch (config) {
	case 0:
		LPC_PINCON->PINSEL0 &= ~((3 << 4) | (3 << 6));
		LPC_PINCON->PINSEL0 |= ((1 << 4) | (1 << 6)); /* RxD0 is P0.3 and TxD0 is P0.2 */
		NVIC_EnableIRQ(UART0_IRQn);
		break;
	case 1:
		LPC_PINCON->PINSEL0 &= ~((3 << 30));
		LPC_PINCON->PINSEL0 |= ((1 << 30));
		LPC_PINCON->PINSEL1 &= ~((3 << 0));
		LPC_PINCON->PINSEL1 |= ((1 << 0)); /* Enable RxD1 P2.1, TxD1 P2.0 */
		NVIC_EnableIRQ(UART1_IRQn);
		break;
	case 2:
		LPC_PINCON->PINSEL0 &= ~((3 << 20) | (3 << 22));
		LPC_PINCON->PINSEL0 |= ((1 << 20) | (1 << 22)); /* Enable RxD1 P2.1, TxD1 P2.0 */
		NVIC_EnableIRQ(UART2_IRQn);
		break;
	case 3:
		LPC_PINCON->PINSEL0 &= ~((3 << 0) | (3 << 2));
		LPC_PINCON->PINSEL0 |= ((2 << 0) | (2 << 2)); /* RxD3 is P0.1 and TxD0 is P0.1 */
		NVIC_EnableIRQ(UART3_IRQn);
		break;
	case 5:
		LPC_PINCON->PINSEL4 &= ~((3 << 4) | (3 << 6));
		LPC_PINCON->PINSEL4 |= ((2 << 4) | (2 << 6)); /* RxD0 is P0.3 and TxD0 is P0.2 */
		NVIC_EnableIRQ(UART1_IRQn);
		break;
	case 6:
		LPC_PINCON->PINSEL4 &= ~((3 << 16) | (3 << 18));
		LPC_PINCON->PINSEL4 |= ((2 << 16) | (2 << 18)); /* RxD0 is P0.3 and TxD0 is P0.2 */
		NVIC_EnableIRQ(UART2_IRQn);
		break;
	case 7:
		LPC_PINCON->PINSEL1 &= ~((3 << 18) | (3 << 20));
		LPC_PINCON->PINSEL1 |= ((3 << 18) | (3 << 20)); /* RxD0 is P0.3 and TxD0 is P0.2 */
		NVIC_EnableIRQ(UART3_IRQn);
		break;
	case 11:
		LPC_PINCON->PINSEL9 &= ~((3 << 24) | (3 << 26));
		LPC_PINCON->PINSEL9 |= ((3 << 24) | (3 << 26)); /* RxD0 is P0.3 and TxD0 is P0.2 */
		NVIC_EnableIRQ(UART3_IRQn);
		break;

	default:
		return (0);
	}
	switch (config % 4) {
	case 0:
		((LPC_UART0_TypeDef*) UART)->LCR = 0x83; /* 8 bits, no Parity, 1 Stop bit */
		((LPC_UART0_TypeDef*) UART)->DLM = Fdiv / 256;
		((LPC_UART0_TypeDef*) UART)->DLL = Fdiv % 256;
		((LPC_UART0_TypeDef*) UART)->LCR = 0x03; /* DLAB = 0 */
		((LPC_UART0_TypeDef*) UART)->FCR = 0x07; /* Enable and reset TX and RX FIFO. */
	case 1:
		((LPC_UART1_TypeDef*) UART)->LCR = 0x83; /* 8 bits, no Parity, 1 Stop bit */
		((LPC_UART1_TypeDef*) UART)->DLM = Fdiv / 256;
		((LPC_UART1_TypeDef*) UART)->DLL = Fdiv % 256;
		((LPC_UART1_TypeDef*) UART)->LCR = 0x03; /* DLAB = 0 */
		((LPC_UART1_TypeDef*) UART)->FCR = 0x07; /* Enable and reset TX and RX FIFO. */
	case 2:
	case 3:
		((LPC_UART_TypeDef*) UART)->LCR = 0x83; /* 8 bits, no Parity, 1 Stop bit */
		((LPC_UART_TypeDef*) UART)->DLM = Fdiv / 256;
		((LPC_UART_TypeDef*) UART)->DLL = Fdiv % 256;
		((LPC_UART_TypeDef*) UART)->LCR = 0x03; /* DLAB = 0 */
		((LPC_UART_TypeDef*) UART)->FCR = 0x07; /* Enable and reset TX and RX FIFO. */
	}
	//	uart_buf = bufmake(80);
	/*
	 switch(config%4){
	 case 0:
	 ((LPC_UART0_TypeDef*)UART)->IER = IER_RBR | IER_THRE | IER_RLS;
	 case 1:
	 ((LPC_UART1_TypeDef*)UART)->IER = IER_RBR | IER_THRE | IER_RLS;
	 case 2:
	 case 3:
	 ((LPC_UART_TypeDef*)UART)->IER = IER_RBR | IER_THRE | IER_RLS;
	 }*/

	return (-1);
}

int uart_putchar(char c, FILE *stream) {

	if (c == '\n')
		uart_putchar('\r', stream);
	LPC_UART0->THR = c;
	while (!(LPC_UART0->LSR & (1 << 6)))
		;//TSR && THR are not empty
	return 0;
}
int putst(char* s) {
	while (*s) {
		uart_putchar(*s, NULL);
		s++;

	}
	return 0;
}

#endif /* SERIAL_H_ */