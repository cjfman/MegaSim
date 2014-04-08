// peripherals.h

#include <stdbool.h>

#define MAX_MSG 255

typedef struct Peripheral {
	char* name;
	int wrfd;		// Write pipe file descriptor
	int rdfd;		// Read pipe file descriptor
	int cpid;
	volatile bool live;
} Peripheral;

Peripheral **peripherals;
int num_perphs;
bool *perph_errors;
extern bool perph_flag;

int openPeripherals(char** perphs, int p_count);
int closePeripherals(void);
void forceClosePeripherals(void);
void startPeripherals(void);
