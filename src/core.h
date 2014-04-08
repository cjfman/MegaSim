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

// Listeners
bool gl_flag; 			// Global listener flag
Peripheral **mem_listeners;	// Array of memory listeners
Peripheral **pin_listeners;	// Array of pin listeners

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

//////////////
// Printing
//////////////

uint8_t stderr_addr;
uint8_t stdout_addr;

#ifdef __cplusplus
}
#endif	// __cplusplus

#endif // CORE_h
