#include "Atomic.hh"
extern "C" {
	#include "LPC17xx.h"
}

void Atomic(EN_DIS_INT int_dint) {
	switch (int_dint) {
	case ENABLE_INT:
		__enable_irq();
		interupt_status::interrupts_enabled = ENABLED_INT;
		return;
	case DISABLE_INT:
		__disable_irq();
		interupt_status::interrupts_enabled = DISABLED_INT;
		return;
	case BEGIN_INT:
		__disable_irq();
		return;
	case RESTORE_INT:
		if (interupt_status::interrupts_enabled == ENABLED_INT)
			__enable_irq();
		return;
	}
}
