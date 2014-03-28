// core.c
//
// Developed by Charles Franklin
//
// MIT Lisense
//

#include "core.h"
#include "handlers.h"

// Initialize pointers
uint16_t *regps = (uint16_t*)regs; // Register pairs
uint16_t *RW = (uint16_t*)(&regs[24]);  // R25:R24
uint16_t *RX = (uint16_t*)(&regs[26]);  // R27:R26
uint16_t *RY = (uint16_t*)(&regs[28]);  // R29:R28
uint16_t *RZ = (uint16_t*)(&regs[30]);  // R31:R30

// Main Control Loop
int runAVR(void) {
	while (true) {
		Instruction inst = program->instructions[pc];
		int error = handlers[inst.op](&inst);
		if (error) {
			return error;
		}
		pc++;
	}
	return 0;
}
