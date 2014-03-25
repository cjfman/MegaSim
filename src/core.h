// Core.h

// Developed by Charles Franklin
//
// MIT Lisense

#ifndef CORE_h
#define CORE_h

#include <inttypes.h>
#include <stdbool.h>
#include "devices.h"

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

// General Perpose Registers
uint8_t regs[32];
extern uint16_t *regps; // Register pairs

// Program Counter
uint32_t pc;

// Main Memory
uint8_t *main_mem;

// I/O Memory
uint8_t *io_mem;

// Program Memory
typedef struct Program {
    uint16_t *data;
    int size;
} Program;

Program *prog_mem;

// Indirect Registers and Poiners
extern uint16_t *RW;	// R25:R24
extern uint16_t *RX;	// R27:R26
extern uint16_t *RY;	// R29:R28
extern uint16_t *RZ;	// R31:R30
uint8_t *EIND;
uint32_t *SP;

////////////////////////
// Device Properties
////////////////////////

coreType core;

#endif
