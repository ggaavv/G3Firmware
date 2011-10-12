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

void Atomic(EN_DIS_INT int_dint);

#endif // ATOMIC_HH
