// core.c
//
// Developed by Charles Franklin
//
// MIT Lisense
//

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "core.h"
#include "handlers.h"
#include "decoder.h"
#include "peripherals.h"

#undef DEBUG

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
	io_mem = main_mem + 0x0020;
	SP   = (uint16_t*)(main_mem + coredef->SP_addr);
	SPL  = (uint8_t*) SP;
	SPH  = SPL + 1;
	EIND = main_mem + coredef->EIND_addr;
	// Listeners
	gl_flag = false;
	mem_listeners = 
		(Peripheral**)malloc(coredef->mem_size*sizeof(Peripheral*));
	memset(mem_listeners, 0, coredef->mem_size*sizeof(Peripheral*));
}

void teardownMemory(void) {
	free(main_mem);
}

// Main Control Loop
int runAVR(void) {
	reset_run();
	while (true) {
		Instruction inst = program->instructions[pc];
#ifdef DEBUG
		fprintf(stderr, "0x%x\t", pc);
		printInstruction(&inst);
		fprintf(stderr, "\n");
#endif // DEBUG
		int error = handlers[inst.op](&inst);
		if (error < 0) {
			return error;
		}
		if (pc >= program->size) {
			printf("%d:%d", program->size, pc);
			return PC_ERROR;
		}
	}
	return 0;
}

void writeMem(uint16_t addr, uint8_t data) {
	if (addr >= coredef->sram_start) {
		main_mem[addr] = data;
	}
	else if (addr == coredef->sreg_addr) {
		sreg[0] = data & 0x1;
		sreg[1] = (data >> 1) & 0x1;
		sreg[2] = (data >> 2) & 0x1;
		sreg[3] = (data >> 3) & 0x1;
		sreg[4] = (data >> 4) & 0x1;
		sreg[5] = (data >> 5) & 0x1;
		sreg[6] = (data >> 6) & 0x1;
		sreg[7] = data >> 7;
	}
	else if (addr == stderr_addr) {
		fprintf(stderr, "%c", data);
	}
	else if (addr == stdout_addr) {
		fprintf(stdout, "%c", data);
	}
	else {
		main_mem[addr] = data;
	}
	// Check for listeners
	if (mem_listeners[addr]) {
		memoryNotification(mem_listeners[addr], addr, data);
	}
}

uint8_t readMem(uint16_t addr) {
	if (addr >= coredef->sram_start) {
		return main_mem[addr];
	}
	if (addr == coredef->sreg_addr) {
		return sreg[0] 
			| (sreg[1] << 1)
			| (sreg[2] << 2)
			| (sreg[3] << 3)
			| (sreg[4] << 4)
			| (sreg[5] << 5)
			| (sreg[6] << 6)
			| (sreg[7] << 7);
	}
	return main_mem[addr];
}
