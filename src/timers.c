// timers.c

#ifndef NO_HARDWARE

#include "timers.c"
#include "core.c"

// Accessing TCCR
#define CS(timer)   (*timer->TCCRB & 0x07)
#define WGM(timer)  (((*timer->TCCRB & 0x08) >> 1) | (*timer->TCCRA & 0x03))
#define FOC(timer)  ((*timer->TCCRB >> 6) & 0x03)
#define COMA(timer) ((*timer->TCCRA >> 6) & 0x03)
#define COMB(timer) ((*timer->TCCRB >> 4) & 0x03)

// Accessing TCCRA
#define WGMn0(timer)    (*timer->TCCRA & 0x01)
#define WGMn1(timer)    ((*timer->TCCRA >> 1) & 0x01)
#define COMnB0(timer)   ((*timer->TCCRA >> 4) & 0x01)
#define COMnB1(timer)   ((*timer->TCCRA >> 5) & 0x01)
#define COMnA0(timer)   ((*timer->TCCRA >> 6) & 0x01)
#define COMnA1(timer)   ((*timer->TCCRA >> 7) & 0x01)

#define setWGMn0(timer)     (*timer->TCCRA | 0x01)
#define setWGMn1(timer)     (*timer->TCCRA | (0x01 << 1))
#define setCOMnB0(timer)    (*timer->TCCRA | (0x01 << 4))
#define setCOMnB1(timer)    (*timer->TCCRA | (0x01 << 5))
#define setCOMnA0(timer)    (*timer->TCCRA | (0x01 << 6))
#define setCOMnA1(timer)    (*timer->TCCRA | (0x01 << 7))

#define clrWGMn0(timer)     (*timer->TCCRA & ~0x01)
#define clrWGMn1(timer)     (*timer->TCCRA & ~(0x01 << 1))
#define clrCOMnB0(timer)    (*timer->TCCRA & ~(0x01 << 4))
#define clrCOMnB1(timer)    (*timer->TCCRA & ~(0x01 << 5))
#define clrCOMnA0(timer)    (*timer->TCCRA & ~(0x01 << 6))
#define clrCOMnA1(timer)    (*timer->TCCRA & ~(0x01 << 7))

// Accessing TCCRB
#define CSn0(timer)     (*timer->TCCRB & 0x01)
#define CSn1(timer)     ((*timer->TCCRB >> 1) & 0x01)
#define CSn2(timer)     ((*timer->TCCRB >> 2) & 0x01)
#define WGMn2(timer)    ((*timer->TCCRB >> 3) & 0x01)
#define FOCnB(timer)    ((*timer->TCCRB >> 6) & 0x01)
#define FOCnA(timer)    ((*timer->TCCRB >> 7) & 0x01)

#define setCSn0(timer)  (*timer->TCCRB | 0x01)
#define setCSn1(timer)  (*timer->TCCRB | (0x01 << 1))
#define setCSn2(timer)  (*timer->TCCRB | (0x01 << 2))
#define setWGMn2(timer) (*timer->TCCRB | (0x01 << 3))
#define setFOCnB(timer) (*timer->TCCRB | (0x01 << 6))
#define setFOCnA(timer) (*timer->TCCRB | (0x01 << 7))

#define clrCSn0(timer)  (*timer->TCCRB & ~0x01)
#define clrCSn1(timer)  (*timer->TCCRB & ~(0x01 << 1))
#define clrCSn2(timer)  (*timer->TCCRB & ~(0x01 << 2))
#define clrWGMn2(timer) (*timer->TCCRB & ~(0x01 << 3))
#define clrFOCnB(timer) (*timer->TCCRB & ~(0x01 << 6))
#define clrFOCnA(timer) (*timer->TCCRB & ~(0x01 << 7))

// Accessing TIFR
#define TOV(timer)     (*timer->TCCRB & 0x01)
#define OCFA(timer)    ((*timer->TCCRB >> 1) & 0x01)
#define OCFB(timer)    ((*timer->TCCRB >> 2) & 0x01)

#define setTOV(timer)  (*timer->TCCRB | 0x01)
#define setOCFA(timer) (*timer->TCCRB | (0x01 << 1))
#define setOCFB(timer) (*timer->TCCRB | (0x01 << 2))

#define clrTOV(timer)  (*timer->TCCRB & ~0x01)
#define clrOCFA(timer) (*timer->TCCRB & ~(0x01 << 1))
#define clrOCFB(timer) (*timer->TCCRB & ~(0x01 << 2))

Timer8bit **timers8bit = NULL;
Timer16bit **timers16bit = NULL;

// Local Functions //////////////////////////////////////////////////

inline handleCOM(uint8_t COM, uint8_t *pin) {
	if (pin == NULL) return;
	switch (COM) {
	case 0:
		break;
	case 1:
		hardWritePin(pin, (pin->hardstate == H) ? L : H);	// Toggle
		break;
	case 2:
		hardWritePin(pin, L);	// Clear
		break;
	case 3:
		hardWritePin(pin, H);	// Set
	}
}


// Timer Management ///////////////////////////////////////////////

void resetTimers(void) {
	prescaler = 0;
	if (tiemrs8bit != NULL) {
		// Loop through timer list
		Timer8bit **timer;
		for (timer = timers8bit; *timer != NULL; timer++) {
			// Set registers to default values
			*(*timer)->TCCRA = 0;
			*(*timer)->TCCRB = 0;
			*(*timer)->TCNT = 0;
			(*timer)->OCRA = 0;
			(*timer)->OCRB = 0;
			*(*timer)->OCRA_buf = 0;
			*(*timer)->OCRB_buf = 0;
			(*timer)->TIFR = 0;
			(*timer)->ext_state = readPin((*timer)->EXT);
			updateTimer8bit(*timer);
		}
	}
	// TODO: 16bit timer reset
	if (timers16bit != NULL) {
		Timer16bit **timer;
		for (timer = timers16bit; *timer != NULL; timer++) {
			updateTimer16bit(*timer);
		}
	}
}


void updateTimer8bit(Timer8bit *timer) {	
	// Set State
	switch (WGM(timer)) {
	case 0:
		timer->state = NORMAL;
		break;
	case 2:
		timer->state = CTC;
		break;
	case 4:	// Reserved
	case 6:	// Reserved
		timer->state = IDLE;
	case 1:
	case 3:
	case 5:
	case 7:
		// TODO: PWM
		break;
	}

	// Check for forced compare
	if (FOCnA(timer)) {
		bool match_a = (*timer->TCNT == timer->OCRA);
		handleCom(COMA(timer), timer->OCA);
		clrFOCnA(timer);
	}
	if (FOCnB(timer)) {
		bool match_b = (*timer->TCNT == timer->OCRB);
		handleCom(COMB(timer), timer->OCB);
		clrFOCnB(timer);
	}

	// Check for timer halt
	if (CS(timer)) {
		timer->state = IDLE;
	}
}

void updateTimer16bit(Timer16bit *timer) {	
}

void runTimers(void) {
	prescaler++;
	if (timers8bit != NULL) {
		// Loop through timer list
		Timer8bit **timer;
		for (timer = timers8bit; *timer != NULL; timer++)  { 
			runTimer8bit(*timer);
		}
	}
	if (timers16bit != NULL) {
		// Loop through timer list
		Timer16bit **timer;
		for (timer = timers16bit; *timer != NULL; timer++)  { 
			runTimer16bit(*timer);
		}
	}
}

void runTimer8Bit(Timer8bit *timer) {
	if (timer->state == idle) return;
	
	// Do compare match on current value
	bool match_a = false;
	bool match_b = false;
	if (timer->state < PWMu && !timer->TCNT_wr) {
		// Timer is not doing PWM
		match_a = (*timer->TCNT == timer->OCRA);
		match_b = (*timer->TCNT == timer->OCRB);
	}

	// Increment on clock source
	bool incr = false;
	case (CS(timer)) {
	case 0:		// Timer stopped
		break;
	case 1:		// No prescale
		incr = 1;
		break;
	case 2:		// clk/8 prescale
		incr = ((prescale & 0x3) == 0);
		break;
	case 3:		// clk/64
		incr = ((prescale & 63) == 0);
		break;
	case 4:		// clk/256
		incr = ((prescale & 255) == 0);
		break;
	case 5:		// clk/1024
		incr = ((prescale & 1023) == 0);
		break;
	case 6:		// EXT pin falling edge
		incr = (timer->ext_state && !readPin(timer->EXT));
		break;
	case 7:		// EXT pin rising edge
		incr = (!timer->ext_state && readPin(timer->EXT));
		break;
	}
	timer->TCNT += incr;

	switch (timer->state) {
	case IDLE:
		break;
	case NORMAL:
		if (match_a) {
			handleCom(COMA(timer), timer->OCA);
		}
		if (match_b) {
			handleCom(COMB(timer), timer->OCB);
		}
		break;
	case CTC:
		if (match_a) {
			timer->TCNT = 0;	// Reset timer
			handleCom(COMA(timer), timer->OCA);
		}
		if (match_b) {
			handleCom(COMB(timer), timer->OCB);
		}
		break;
	case PWMu:
	case PWMd:
	case PWMf:
	}

	// Set interrupt flags
	if (match_a) setOCFA(timer);
	if (match_b) setOCFB(timer);
	if (timer->TCNT == 0) setTOV(timer);
	timer->TCNT_wr = false;
}

#endif 	// NO_HARDWARE
