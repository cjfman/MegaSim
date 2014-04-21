// timers.h

#ifndef TIMERS_h
#define TIMERS_h

#ifdef NO_HARDWARE
#define NO_TIMERS
#endif	// NO_HARDWARE

#ifndef NO_TIMERS

#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <core.h>

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

typedef enum TimerState {
	IDLE = 0,
	NORMAL = 1,
	CTC = 2,
	PWMu = 3,
	PWMd = 4,
	PWMf = 5
} TimerState;

typedef struct Timer8bit {
	uint8_t *TCCRA;		// Timer Control A
	uint8_t *TCCRB;		// Timer Control B
	uint8_t *TCNT;		// Timer/Counter
	bool TCNT_wr;		// TCNT register was written this cycle
	uint8_t OCRA;		// Output Compare A
	uint8_t OCRB;		// Output Compare B
	uint8_t *OCRA_buf;	// Output Compare A buffer
	uint8_t *OCRB_buf;	// Output Compare B buffer
	uint8_t *TIFR;		// Interrupt Flag Register
	TimerState state;	// The state of the timer
	uint8_t ext_state;	// The state of the external clk source
	uint8_t OCA;		// Pins for wave generation
	uint8_t OCB;
	uint8_t EXT;		// External trigger pin
} Timer8bit;

extern **Timer8bit timers8bit;	// Null terminated list of timers

typedef struct Timer16bit {
	uint8_t TCCRA;	// Timer Control A
	uint8_t TCCRB;	// Timer Control B
	uint8_t TCNT;	// Counter low byte
	uint8_t OCRAL;	// Output Compare A low byte
	uint8_t OCRBL;	// Output Compare B low byte
	uint8_t TCCRC; 	// Timer Control C
	uint8_t TCNTH;	// Counter high byte
	uint8_t ICRL;	// Input Compare low byte
	uint8_t ICRH;	// Input Compare high byte
	uint8_t OCRAH;	// Output Compare A high byte
	uint8_t OCRBH;	// Output Compare B high byte
	uint8_t OCRCL;	// Output Compare C low byte
	uint8_t OCRCH;	// Output Compare C high byte
} Timer16bit;

extern **Timer16bit timers16bit;	// Null terminated list of timers

uint32_t prescaler;

void resetTimers(void);
void updateTimer8bit(Timer8bit *timer);
void updateTimer16bit(Timer16bit *timer);
void runTimers(void);
void runTimer8bit(Timer8bit *timer);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif	// NO_TIMERS

#endif 	// TIMERS_h
