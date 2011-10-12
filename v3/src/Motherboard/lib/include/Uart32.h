
#ifndef UART32_H_
#define UART32_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lpc17xx_uart.h"
#include "LPC17xx.h"

void UART_8(LPC_UART_TypeDef* UARTx, uint8_t Data);
void UART_32_DEC(LPC_UART_TypeDef* UARTx, uint32_t Data);
void UART_32_HEX(LPC_UART_TypeDef* UARTx, uint32_t Data);

#ifdef __cplusplus
}
#endif

#endif /* UART32_H_ */

