// peripherals.h

#ifndef NO_PERPHS
#ifndef PERIPHERAL_h
#define PERIPHERAL_h

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <inttypes.h>
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
void memoryNotification(Peripheral* perph, uint16_t addr, uint8_t data);
void portNotification(Peripheral* perph, uint16_t port, uint8_t data);
void pinNotification(Peripheral* perph, uint16_t pin, uint8_t data);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif // PERIPHERAL_h

#endif // NO_PERPHS
