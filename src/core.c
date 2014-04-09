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

	// Ports and Pins
	pins = (Pin**)malloc(coredef->num_pins*sizeof(Pin*));
	memset(pins, 0, coredef->num_pins*sizeof(Pin*));
	uint16_t port_addrs[11] = {coredef->port_a,
							   coredef->port_b,
							   coredef->port_c,
							   coredef->port_d,
							   coredef->port_e,
							   coredef->port_f,
							   coredef->port_g,
							   coredef->port_h,
							   coredef->port_j,
							   coredef->port_k,
							   coredef->port_l};
	uint8_t *port_maps[11] = {   coredef->port_a_map,
							     coredef->port_b_map,
							     coredef->port_c_map,
							     coredef->port_d_map,
							     coredef->port_e_map,
							     coredef->port_f_map,
							     coredef->port_g_map,
							     coredef->port_h_map,
							     coredef->port_j_map,
							     coredef->port_k_map,
							     coredef->port_l_map};
	int i;
	for (i = 0; i < 11; i++) {
		// Check to see if port exists
		if (port_addrs[0] == 0) {
			ports[i] = NULL;
			continue;
		}
		// Make new port object and populate its pointers
		Port *port = (Port*)malloc(sizeof(Port));
		port->pin  = main_mem + port_addrs[i];
		port->ddr  = main_mem + port_addrs[i] + 1;
		port->port = main_mem + port_addrs[i] + 2;
		port->claim = false;
		port->pin_claim = false;
		int j;
		for (j = 0; j < 8; j++) {
			// Initialize all mapped pins
			port->pin_map[j] = port_maps[i][j] - 1;
			if(port_maps[i][j] == 0) continue;
			Pin *pin = (Pin*)malloc(sizeof(Pin));
			pin->port = port;
			pin->bit = j;
			pin->state = Z;
			pin->claim = false;
			pins[port_maps[i][j] - 1] = pin;
		}
		ports[i] = port;
	}

	// Listeners
	gl_flag = false;
	mem_listeners = 
		(Peripheral**)malloc(coredef->mem_size*sizeof(Peripheral*));
	memset(mem_listeners, 0, coredef->mem_size*sizeof(Peripheral*));
	port_listeners = (Peripheral**)malloc(11*sizeof(Peripheral*));
	memset(port_listeners, 0, 11*sizeof(Peripheral*));
	pin_listeners = (Peripheral**)malloc(coredef->num_pins*sizeof(Peripheral*));
	memset(pin_listeners, 0, coredef->num_pins*sizeof(Peripheral*));
}

void teardownMemory(void) {
	free(main_mem);
	main_mem = NULL;
	int i;
	for (i = 0; i < 11; i++) {
		free(ports[i]);
		ports[i] = NULL;
	}
	for (i = 0; i < coredef->num_pins; i++) {
		if (pins[i]) {
			free(pins[i]);
			pins[i] = NULL;
		}
	}
	free(pins);
	pins = NULL;
	free(mem_listeners);
	mem_listeners = NULL;
	free(port_listeners);
	port_listeners = NULL;
	free(pin_listeners);
	pin_listeners = NULL;
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

uint8_t masks[8] = { 0x01,
					 0x02,
					 0x04,
					 0x08,
					 0x10,
					 0x20,
					 0x40,
					 0x80 };

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
	// Optimize for ram access
	if (addr >= coredef->sram_start) {
		return main_mem[addr];
	}
	// Optimize certain address ranges
	if (addr >= 0x58 || addr <= 0x5E) {
		return main_mem[addr];
	}
	// Check for status register
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
	// Check for ports
	int i;
	for (i = 0; i < 11; i++) {
		Port *port = ports[i];
		if (port == NULL || (main_mem + addr) != port->pin) continue;
		if (port->pin_claim) {
			// There is a peripheral that can write to a pin on this port
			// Each pin must be checked for it's status
			uint8_t res = 0;
			int j;
			for (j = 0; j < 8; j++) {
				// Loop through each pin
				// The value of unmapped bits will be 0
				if (port->pin_map[j] == -1) continue;	// Bit is unmapped. Ignore.
				PinState state = (pins[port->pin_map[j]]->state);
				// If the pin is in output mode, use the internal driving value
				// If the pin is in input mode and is floating, check for pullup
				// If no pullup, value will be low
				// Else, use the externally driven value
				res |= (*port->ddr & masks[j]) ? *port->port & masks[j] :
					   (state == Z)            ? *port->port & masks[j] :
					                                         state << j ;
			}
			return res;
		}
		// No peripheral can write to this port
		// Simply use PORTx
		return *port->port;
	}
	return main_mem[addr];
}

PinState readPin(uint8_t pin_num) {
	if (pin_num > coredef->num_pins) return X;		// This pin does not exist
	Pin *pin = pins[pin_num - 1];
	if (pin == NULL) return X;						// This pin is not mapped
	Port *port = pin->port;
	// Value if the state if output
	PinState out = (*port->port & masks[pin->bit]) >> pin->bit;	
	// Value if the state is input and undriven
	PinState in = (out) ? H : Z;
	return (*port->ddr & masks[pin->bit]) ? out        :
		   (pin->claim)                   ? pin->state :
		                                    in         ;
}
