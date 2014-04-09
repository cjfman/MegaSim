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
#include "peripherals.h"

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
	bool claim;			// Set if a listener my write to this port
	bool pin_claim;		// Set if any of the pins may be written
} Port;

Port *ports[11];

typedef enum PinState {
	Z,		// Undriven
	X,		// Nonsense
	H = 1,	// High
	L = 0	// Low
} PinState;

typedef struct Pin {
	Port *port;			// A pointer to the port
	uint8_t bit;		// The bit index in that port
	PinState state;		// The current driven state of the pin
	bool claim;			// Set if a listener my write to this pin
} Pin;

Pin **pins;

// Listeners
bool gl_flag; 					// Global listener flag
Peripheral **mem_listeners;		// Array of memory listeners
Peripheral **port_listeners;	// Array of port listeners
Peripheral **pin_listeners;		// Array of pin listeners
bool *mem_claim;	// Boolean array. Set true if a listener may write this address

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
PinState readPin(uint8_t pin_num);

//////////////
// Printing
//////////////

uint8_t stderr_addr;
uint8_t stdout_addr;

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif // CORE_h
