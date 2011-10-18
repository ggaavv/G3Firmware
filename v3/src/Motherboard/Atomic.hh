#ifndef ATOMIC_HH
#define ATOMIC_HH

enum EN_DIS_INT {
	ENABLE_INT,
	DISABLE_INT,
	ENABLED_INT,
	DISABLED_INT,
	BEGIN_INT,
	RESTORE_INT
};

namespace interupt_status {
EN_DIS_INT interrupts_enabled = DISABLED_INT;
}

void Atomic(EN_DIS_INT int_dint);

#endif // ATOMIC_HH
