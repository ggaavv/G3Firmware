extern "C" {
	#include "LPC17xx.h"
	#include "lpc17xx_timer.h"
}

void Delay_us (uint32_t us_delay) {
	// start timer
	// interrupt timer.
	TIM_TIMERCFG_Type TMR3_Cfg;
	TIM_MATCHCFG_Type TMR3_Match;
	TMR3_Cfg.PrescaleOption = TIM_PRESCALE_USVAL;
	TMR3_Cfg.PrescaleValue = 1;
	/* Use channel 1, MR1 */
	TMR3_Match.MatchChannel = 0;
	/* Enable interrupt when MR0 matches the value in TC register */
	TMR3_Match.IntOnMatch = DISABLE;
	/* Enable reset on MR0: TIMER will reset if MR0 matches it */
	TMR3_Match.ResetOnMatch = TRUE;
	/* Don't stop on MR0 if MR0 matches it*/
	TMR3_Match.StopOnMatch = TRUE;
	/* Do nothing for external output pin if match (see cmsis help, there are another options) */
	TMR3_Match.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	/* Set Match value, count value of INTERVAL_IN_MICROSECONDS (64 * 1uS = 64us = 1s --> 15 kHz) */
	TMR3_Match.MatchValue = us_delay;
	/* Set configuration for Tim_config and Tim_MatchConfig */
	TIM_Init(LPC_TIM3, TIM_TIMER_MODE, &TMR3_Cfg);
	TIM_ConfigMatch(LPC_TIM3, &TMR3_Match);
	while (TIM_GetIntStatus(LPC_TIM3, TIM_MR0_INT) == 0);
}
