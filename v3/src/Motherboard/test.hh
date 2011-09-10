#include <stdint.h>
extern "C" {
	#include "lpc17xx_uart.h"
	#include "lpc17xx_pinsel.h"
	#include "debug_frmwrk.h"
}

#define UART_PORT 0

#if (UART_PORT == 0)
#define TEST_UART LPC_UART0
#elif (UART_PORT == 1)
#define TEST_UART (LPC_UART_TypeDef *)UART1
#endif

/************************** PRIVATE VARIABLES *************************/
uint8_t menu1[] = "4567";
uint8_t menu2[] = "UART";
uint8_t menu3[] = "UAR!";

void print_menu(void)
{
	UART_Send(TEST_UART, menu1, sizeof(menu2), BLOCKING);
	UART_Send(TEST_UART, menu2, sizeof(menu2), BLOCKING);
	UART_Send(TEST_UART, menu3, sizeof(menu3), BLOCKING);
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
	UART_Init(TEST_UART, &UARTConfigStruct);

	/* Initialize FIFOConfigStruct to default state:
	 * 				- FIFO_DMAMode = DISABLE
	 * 				- FIFO_Level = UART_FIFO_TRGLEV0
	 * 				- FIFO_ResetRxBuf = ENABLE
	 * 				- FIFO_ResetTxBuf = ENABLE
	 * 				- FIFO_State = ENABLE
	 */
	UART_FIFOConfigStructInit(&UARTFIFOConfigStruct);

	// Initialize FIFO for UART0 peripheral
	UART_FIFOConfig(TEST_UART, &UARTFIFOConfigStruct);


	// Enable UART Transmit
	UART_TxCmd(TEST_UART, ENABLE);

	// print welcome screen
	print_menu();
	uint8_t a = 4;
	uint8_t b = 5;
	uint8_t c = 6;
	uint8_t d = 7;

	UART_SendByte(TEST_UART, a);
	UART_SendByte(TEST_UART, b);
	UART_SendByte(TEST_UART, c);
	UART_SendByte(TEST_UART, d);

	while(1);
}
