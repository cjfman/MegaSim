// Core.h

// Developed by Charles Franklin
//
// MIT Lisense

#ifndef CORE_h
#define CORE_h

#include <inttypes.h>
#include "bool.h"
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
uint16_t *regps = (uint16_t*)regs; // Register pairs

// Program Counter
uint32_t pc;

// Main Memory
uint8_t *main_mem;

// I/O Memory
uint8_t *io_mem;

// Program Memory
typedef struct Program Program;

Program *prog_mem;

// Indirect Registers and Poiners
uint16_t *RW = (uint16_t*)(&regs[24]);	// R25:R24
uint16_t *RX = (uint16_t*)(&regs[26]);	// R27:R26
uint16_t *RY = (uint16_t*)(&regs[28]);	// R29:R28
uint16_t *RZ = (uint16_t*)(&regs[30]);	// R31:R30
uint8_t *EIND;
uint32_t *SP;

////////////////////////
// Device Properties
////////////////////////

coreType core;
