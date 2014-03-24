// decoder.h

// Developed by Charles Franklin
//
// MIT Lisense

#ifndef DECODER_h
#define DECODER_h

#include <inttypes.h>
#include "bool.h"
#include "core.h"
#include "devices.h"

#ifndef IS_ARITHMETIC_RS 
#define IS_ARITHMETIC_RS (-2 >> 1 == -1)
#endif

////////////////////////
// Opt Code Decoding
////////////////////////

// Stores decoded instructions
typedef struct Instuction Instruction;

// Make a new blank instruction
void makeBlankIntruction(Instruction *inst);

// Decodes instructions
void decodedInstruction(Instruction *inst, uint16_t opcode);


/////////////////////////////////////
// Error Codes
/////////////////////////////////////

#define BAD_REG 1

#endif
