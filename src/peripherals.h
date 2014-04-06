// peripherals.h

#include <stdbool.h>

typedef struct Peripheral {
	char* name;
	int wr_pipefd;
	int rd_pipefd;
	int cpid;
	volatile bool live;
} Peripheral;

Peripheral **peripherals;
int num_perphs;
int *perph_errors;
extern int perph_flag;

int openPeripherals(char** perphs, int p_count);
int closePeripherals(void);
