// core.c
//
// Developed by Charles Franklin
//
// MIT Lisense
//

#include <stdlib.h>
#include "core.h"
#include "handlers.h"

void setupMemory(void) {
	// Initialize pointers
	// Main Memory
	main_mem = (uint8_t*)malloc(coredef->mem_size);
	regs = main_mem;
	regps = (uint16_t*)regs; 		// Register pairs
	RW = (uint16_t*)(&regs[24]);  	// R25:R24
	RX = (uint16_t*)(&regs[26]);  	// R27:R26
	RY = (uint16_t*)(&regs[28]);  	// R29:R28
	RZ = (uint16_t*)(&regs[30]);  	// R31:R30

	// IO Memory
	io_mem = (uint8_t*)malloc(coredef->io_mem_size);
	SP   = io_mem + coredef->SP_addr;
	EIND = io_mem + coredef->EIND_addr;
}

// Main Control Loop
int runAVR(void) {
	while (true) {
		Instruction inst = program->instructions[pc];
		int result = handlers[inst.op](&inst);
		if (result < 0) {
			return result;
		}
		pc += result;
	}
	return 0;
}
