// Core.h
//
// Developed by Charles Franklin
//
// MIT Lisense

#ifndef CORE_h
#define CORE_h

#include <inttypes.h>

////////////////////////
// Opt Code Decoding
////////////////////////

typedef enum IReg {
	RX,
	RY,
	RZ
} IReg;

// Stores decoded instructions
typedef struct Instuction {
	uint8_t op;		// The basic operation
	uint8_t R;		// The source register 
	uint8_t D;		// The destination/source register
	uint8_t mode;		// Mode of the operation
	uint8_t A;		// Address
	int8_t K;		// Constant
	IReg ireg;		// Indirect register
} Instruction;

// Decodes instructions
decodedInstruction(Instruction *inst, uint16_t opcode);

#endif
