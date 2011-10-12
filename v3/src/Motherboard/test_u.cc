
#include "test_u.hh"

extern "C" {
	#include "lpc17xx_pinsel.h"
	#include "LPC17xx.h"
	#include "lpc17xx_uart.h"
}

void test_u(void){
	UART_CFG_Type u_cfg;
	u_cfg.Baud_rate = 512000;
	u_cfg.Databits = UART_DATABIT_8;
	u_cfg.Parity = UART_PARITY_NONE;
	u_cfg.Stopbits = UART_STOPBIT_1;
	UART_Init((LPC_UART_TypeDef *)LPC_UART2, &u_cfg);
	// Initialize UART2 pin connect
	PINSEL_CFG_Type PinCfg;
	PinCfg.Funcnum = 1;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 10;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&PinCfg);
	UART_TxCmd((LPC_UART_TypeDef *)LPC_UART2, ENABLE);
};
