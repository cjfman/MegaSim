// core.h

// Developed by Charles Franklin
//
// MIT Lisense

#ifndef CORE_h
#define CORE_h

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

#include <inttypes.h>
#include <stdbool.h>
#include "devices.h"
#include "decoder.h"

#ifndef NO_PERPHS
#include "peripherals.h"
#endif // NO_PERPHS

// MACROS
#define GETBIT(var, index) ((var >> index) 0x01)

////////////////////////
// Memory Structures
////////////////////////

// Status register
// +-------------------------------+
// | I | T | H | S | V | N | Z | C |
// +-------------------------------+
//  MSB                         LSB
#define CREG 0 // C: Carry Flag
#define ZREG 1 // Z: Zero Flag
#define NREG 2 // N: Negative Flag
#define VREG 3 // V: Two's complement overflow indicator
#define SREG 4 // S: N XOR V, for signed tests
#define HREG 5 // H: half Carry Flag
#define TREG 6 // T: Tranfer bit
#define IREG 7 // I: Global Interrupt Enable/Disable Flag
bool sreg[8];


// Program Counter
uint32_t pc;

// Main Memory
uint8_t *main_mem;

// General Perpose Registers
uint8_t *regs;
uint16_t *regps; // Register pairs

// I/O Memory
uint8_t *io_mem;

// Program Memory
typedef struct Program {
    uint16_t *data;
	Instruction *instructions;
    unsigned int size;	// In words
} Program;

Program *program;

// Indirect Registers and Poiners
uint16_t *RW;	// R25:R24
uint16_t *RX;	// R27:R26
uint16_t *RY;	// R29:R28
uint16_t *RZ;	// R31:R30
uint8_t *EIND;
uint16_t *SP;	// Stack pointer
uint8_t *SPH;	// Stack pointer high
uint8_t *SPL;	// Stack pointer low

#ifndef NO_PORTS
// I/O Pins
#define PORTA 0
#define PORTB 1
#define PORTC 2
#define PORTD 3
#define PORTE 4
#define PORTF 5
#define PORTG 6
#define PORTH 7
#define PORTJ 8
#define PORTK 9
#define PORTL 10

typedef struct Port {
	uint8_t *pin;		// Address of PINx
	uint8_t *ddr;		// Address of DDRx
	uint8_t *port;		// Address of PORTx
	int8_t pin_map[8];	// The mapping of pins to indexes
#ifndef NO_PERPHS
	Peripheral *listener;
	bool pin_listener;	// There is a listener on at least one of the pins
#endif
} Port;

Port *ports[11];

typedef enum PinState {
	H = 1,	// High
	L = 0,	// Low
	Z = 2,	// Undriven
	X = 3	// Nonsense
} PinState;

typedef struct Pin {
	Port *port;			// A pointer to the port
	uint8_t bit;		// The bit index in that port
	PinState state;		// The current input drive state of the pin
#ifndef NO_PERPHS
	Peripheral *listener;
#endif
} Pin;

Pin **pins;

#endif // NO_PORTS

// Listeners
#ifndef NO_PERPHS
bool gl_flag; 					// Global listener flag
Peripheral **mem_listeners;		// Array of memory listeners
bool *mem_claim;				// Set true if a listener may write 
								// this address
#endif	// NO_PERPHS

////////////////////////
// Device Properties
////////////////////////

CoreDef *coredef;

/////////////////////////
// Functions
/////////////////////////

void setupMemory(void);
void teardownMemory(void);
int runAVR(void);
void writeMem(uint16_t addr, uint8_t data);
uint8_t readMem(uint16_t addr);
#ifndef NO_PORTS
PinState readPin(uint8_t pin_num);
void writePin(uint8_t pin_num, PinState state);
#endif	// NO_PORTS

//////////////
// Printing
//////////////

uint8_t stderr_addr;
uint8_t stdout_addr;

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif // CORE_h
