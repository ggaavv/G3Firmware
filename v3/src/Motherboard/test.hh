#include <stdint.h>
extern "C" {
	#include "lpc17xx_uart.h"
	#include "lpc17xx_pinsel.h"
}


/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] = "TESTING 123";
uint8_t menu2[] = "Makerbot LPC1768";
uint8_t menu3[] = "TESTING 123";

void print_menu(void)
{
	UART_Send(LPC_UART0, menu1, sizeof(menu1), BLOCKING);
	UART_Send(LPC_UART0, menu2, sizeof(menu2), BLOCKING);
	UART_Send(LPC_UART0, menu3, sizeof(menu3), BLOCKING);
}

void test_uart(){
// UART Configuration structure variable
	UART_CFG_Type UARTConfigStruct;
	// UART FIFO configuration Struct variable
	UART_FIFO_CFG_Type UARTFIFOConfigStruct;
	// Pin configuration for UART0
	PINSEL_CFG_Type PinCfg;

	/*
	 * Initialize UART0 pin connect
	 */
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 2;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 3;
	PINSEL_ConfigPin(&PinCfg);


	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	UART_ConfigStructInit(&UARTConfigStruct);

	// Initialize UART0 peripheral with given to corresponding parameter
	UART_Init(LPC_UART0, &UARTConfigStruct);

	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig(LPC_UART0, &UARTFIFOConfigStruct);


	// Enable UART Transmit
	UART_TxCmd(LPC_UART0, ENABLE);

	// print welcome screen
	print_menu();
	uint8_t a = 4;
	uint8_t b = 5;
	uint8_t c = 6;
	uint8_t d = 7;

	UART_SendByte(LPC_UART0, a);
	UART_SendByte(LPC_UART0, b);
	UART_SendByte(LPC_UART0, c);
	UART_SendByte(LPC_UART0, d);

	//while(1);
}
