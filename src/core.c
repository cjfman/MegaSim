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
#include <time.h>
#include "core.h"
#include "handlers.h"
#include "decoder.h"
#include "peripherals.h"
#include "debugterm.h"

#ifdef NO_HARDWARE
#define NO_PORTS
#define NO_PERPHS
#endif	// NO_HARDWARE

#ifdef DEBUG
#define DEBUG_CORE
#endif //DEBUG

//bool sreg[8];

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
	EIND =  main_mem + coredef->EIND_addr;
	RAMPZ = main_mem + coredef->RAMPZ_addr;

#ifndef NO_PORTS
	// Ports and Pins
	pins = (Pin**)malloc(coredef->num_pins*sizeof(Pin*));
	memset(pins, 0, coredef->num_pins*sizeof(Pin*));
	int i;
	// Allocate memory for ports
	for (i = 0; coredef->ports[i] != 0; i++);	// Count ports
	num_ports = i;
	ports = (Port**)malloc(num_ports*sizeof(Port*));
	
	for (i = 0; i < num_ports; i++) {
		// Make new port object and populate its pointers
		Port *port = (Port*)malloc(sizeof(Port));
		port->pin  = main_mem + coredef->ports[i];
		port->ddr  = main_mem + coredef->ports[i] + 1;
		port->port = main_mem + coredef->ports[i] + 2;
#ifndef NO_PERPHS
		port->listener = NULL;
		port->pin_listener = false;
#endif	// NO_PERPHS
		int j;
		for (j = 0; j < 8; j++) {
			// Initialize all mapped pins
			port->pin_map[j] = coredef->port_maps[i][j] - 1;
			if(coredef->port_maps[i][j] == 0) continue;
			Pin *pin = (Pin*)malloc(sizeof(Pin));
			pin->port = port;
			pin->bit = j;
			pin->harddrive = false;
			pin->state = Z;
#ifndef NO_PERPHS
			pin->listener = NULL;
#endif	// NO_PERPHS
			pins[coredef->port_maps[i][j] - 1] = pin;
		}
		ports[i] = port;
	}
#endif // NO_PORTS

	// Listeners
#ifndef NO_PERPHS
	gl_flag = false;
	mem_listeners = 
		(Peripheral**)malloc(coredef->mem_size*sizeof(Peripheral*));
	memset(mem_listeners, 0, coredef->mem_size*sizeof(Peripheral*));
#endif
}

void teardownMemory(void) {
	free(main_mem);
	main_mem = NULL;
#ifndef NO_PORTS
	int i;
	for (i = 0; i < num_ports; i++) {
		free(ports[i]);
		ports[i] = NULL;
	}
	free(ports);
	ports = NULL;
	for (i = 0; i < coredef->num_pins; i++) {
		if (pins[i]) {
			free(pins[i]);
			pins[i] = NULL;
		}
	}
	free(pins);
	pins = NULL;
#endif // NO_PORTS
#ifndef NO_PERPHS
	free(mem_listeners);
	mem_listeners = NULL;
#endif
}

// Main Control Loop
int runAVR(void) {
	reset_run();
	while (true) {
		// Load instruction
		Instruction inst = program->instructions[pc];	
#ifdef DEBUG_CORE
		fprintf(stderr, "0x%x\t", pc);
		printInstruction(&inst);
		fprintf(stderr, "\n");
#endif // DEBUG_CORE
		// Run instruction
		int error;
		if (break_now) {
			break_now = false;
			error = runDebugTerm();
		}
		else {
			error = handlers[inst.op](&inst);			
		}
		if (error != 0) {								
			return error;
		}
		// Check PC for errors
		if (pc >= program->size) {
			printf("%d:%d", program->size, pc);
			return PC_ERROR;
		}
#ifndef NO_HARDWARE
		// Call hardware handler
		int i;
		for (i = 0; i < inst.cycles; i++) {
			cycle_count++; 
		}
#else
		cycle_count += inst.cycles;
#endif 	// NO_HARDWARE
#ifndef NO_PERPHS
		if (perph_write_flag) {
			checkPeripherals();
		}
#endif	// NO_PERPHS
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
	if (addr == coredef->sreg_addr) {
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
#ifndef NO_PORTS
		// Check for ports
		uint8_t* real_addr = main_mem + addr;
		int i;
		// Loop through each port
		for (i = 0; i < num_ports; i++) {
			Port *port = ports[i];
			//if (port == NULL) continue;			// Port does not exist
			if (real_addr == port->pin) break;		// Read only
			if (real_addr == port->port || real_addr == port->ddr) {
				main_mem[addr] = data;					// Save data
				uint8_t set = *port->port;				// Set bits that pull high
														// or drive high
				uint8_t stay = *port->pin & ~*port->ddr;// Set floating bits that
														// are currently high
				*port->pin = set | stay;				// Set the new read values
#ifndef NO_PERPHS
				// Do pin updates and notifications
				int j;
				for (j = 0; j < 8; j++) {
					if (!port->pin_listener && port->listener == NULL) {
						break;
					}
					// Loop through each pin
					if (port->pin_map[j] == -1) {
						continue;					// Ignore unmapped bit
					}
					Pin *pin = pins[port->pin_map[j]];
					bool notify = false;			// Should notify pin listener
					// If the pin is in output mode, notify listener
					if (*port->ddr & masks[j]) {
						notify = true;
					}
					// Else, use the externally driven value
					switch (pin->state) {
					case H:
						// Set bit
						*port->pin |= masks[j];
						break;
					case L:
						// Clear bit
						*port->pin &= ~masks[j];
						break;
					default:
						notify = true;
						break;
					}
					// Notify listener
					if (pin->listener != NULL && notify) {
						uint8_t val = ((*port->port & masks[j]) >> j);
						pinNotification(pin->listener, port->pin_map[j] + 1, val);
					}
				}
				if (port->listener) {
					portNotification(port->listener, i, *port->pin);
					break;	// Do not look at other ports
							// Do not send pin notifications
				}
#endif //  NO_PERPHS
				break;	// Do not look at other ports
			}
		}
		if (i == num_ports) {
			// None of the ports matched
			main_mem[addr] = data;
		}
#else // NO_PORTS
		main_mem[addr] = data;
#endif 	// NO_PORTS
	}

#ifndef NO_PERPHS
	// Check for memory listeners
	if (mem_listeners[addr]) {
		memoryNotification(mem_listeners[addr], addr, data);
	}
#endif // NO_PERPHS
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
	return main_mem[addr];
}

#ifndef NO_PORTS
PinState readPin(uint8_t pin_num) {
	if (pin_num > coredef->num_pins) return X;		// This pin does not exist
	Pin *pin = pins[pin_num - 1];
	if (pin == NULL) return X;						// This pin is not mapped
	Port *port = pin->port;
	int mode = (*port->ddr & masks[pin->bit]);
	if (mode) {
		// Output mode
		if (pin->harddrive) {
			return pin->hardstate;
		}
		return (*port->port & masks[pin->bit]) ? H : L;
	}
	// Input mode
	return (*port->pin & masks[pin->bit]) ? H : L;
}

bool writePin(uint8_t pin_num, PinState state) {
	if (pin_num > coredef->num_pins) 
		return false;
	Pin *pin = pins[pin_num - 1];
	if (pin == NULL) 
		return false;
	pin->state = state;							// Update pin state
	Port *port = pin->port;
	int mode = (*port->ddr & masks[pin->bit]);
	if (!mode) {
		switch (state) {
		case H:
			*port->pin |= masks[pin->bit];			// Set pin 
			break;
		case L:
			*port->pin &= ~masks[pin->bit];			// Clear pin
			break;
		case X:
		case Z:
			*port->pin |= *port->port & masks[pin->bit];	// Set if pullup
			break;											// Otherwise leave unchagned
		}
	}
	/*
	// Notify listener
	Peripheral *perph;
	if (pin->listener != NULL) {
		int bit = pin->bit;
		uint8_t val = ((*port->port & masks[bit]) >> bit);
		pinNotification(pin->listener, port->pin_map[bit] + 1, val);
	}
	// */
	return true;
}	

bool hardWritePin(uint8_t pin_num, PinState state) {
	if (pin_num > coredef->num_pins) {
		return false;
	}
	Pin *pin = pins[pin_num - 1];
	if (pin == NULL) {
		return false;
	}
	pin->hardstate = state;
	Port *port = pin->port;
	if (!pin->harddrive) {
		// Pin is not driven by hardware. Do not notify listener
		return true;
	}
	// Notify listener
	Peripheral *perph;
	if ((perph = pin->listener) != NULL 
		|| ((perph = port->listener) != NULL)) 
	{
		int bit = pin->bit;
		pinNotification(pin->listener, pin_num, state);
	}
	return true;
}

#endif 	// NO_PORTS
