#include "LPC17xx.h"
#include "lpc17xx_nvic.h"

void enable_USB_interrupts(void)
{
	NVIC_SetPriority(USB_IRQn, 2);
	NVIC_EnableIRQ(USB_IRQn);
	TIM_Cmd(LPC_TIM1,ENABLE);
}
