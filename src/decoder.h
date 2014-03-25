// decoder.h

// Developed by Charles Franklin
//
// MIT Lisense

#ifndef DECODER_h
#define DECODER_h

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include "core.h"
#include "devices.h"

#define IS_ARITHMETIC_RS (-2 >> 1 == -1)

////////////////////////
// Opt Code Decoding
////////////////////////

// Stores decoded instructions
typedef struct Instuction {
	uint8_t op;		// The basic operation
	uint8_t R;		// The source register. Sometimes SREG bit index. 
	uint8_t D;		// The destination/source register. Sometimes bit value.
	uint8_t mode;	// Mode of the operation
	uint16_t A;		// Address and offset
	int8_t K;		// Constant
	uint16_t *ireg;	// Indirect register
	uint8_t wsize;	// The size in words of the instruction
} Instruction;

// Make a new blank instruction
void makeBlankInstruction(Instruction *inst);

// Decodes instructions
void decodeInstruction(Instruction *inst, uint16_t opcode);


/////////////////////////////////////
// Error Codes
/////////////////////////////////////

#define BAD_REG 1

#ifdef __cplusplus
}
#endif

#endif // DECODER_h
