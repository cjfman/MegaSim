// MegaTest Devices.h
//
// Developed by Charles Franklnin
//

#ifndef DEVICES_h
#define DEVICES_h

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>

// Core Definition
typedef struct CoreDef {
	uint8_t type;
	unsigned int mem_size;		// Size in bytes
	unsigned int prog_mem_size;	// Size in words
	uint16_t sram_start;		// Address of the start of the SRAM
	uint16_t sram_end;
	unsigned int pc_size;		// Size in bits
	uint16_t sreg_addr;			// Address of the status register
	uint16_t SP_addr;			// The address of the SP in IO space
	uint16_t EIND_addr;
	uint16_t RAMPZ_addr;
	uint8_t num_pins;			// The number of pins
	uint16_t port_a;			// The base address of the port
	uint16_t port_b;			// Every port has three addresses
	uint16_t port_c;			// This address designates PINx
	uint16_t port_d;			// DDRx is the following address
	uint16_t port_e;			// and PORTx after that
	uint16_t port_f;
	uint16_t port_g;
	uint16_t port_h;
	uint16_t port_j;
	uint16_t port_k;
	uint16_t port_l;
	uint8_t port_a_map[8];
	uint8_t port_b_map[8];
	uint8_t port_c_map[8];
	uint8_t port_d_map[8];
	uint8_t port_e_map[8];
	uint8_t port_f_map[8];
	uint8_t port_g_map[8];
	uint8_t port_h_map[8];
	uint8_t port_j_map[8];
	uint8_t port_k_map[8];
	uint8_t port_l_map[8];
} CoreDef;

extern CoreDef default_core;

#ifdef __cplusplus
}
#endif

#endif // DEVICES_h
