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
#include <stdbool.h>

// Port Definitions
#define PORT_A 1
#define PORT_B (1 << 1)
#define PORT_C (1 << 2)
#define PORT_D (1 << 3)
#define PORT_E (1 << 4)
#define PORT_F (1 << 5)
#define PORT_G (1 << 6)
#define PORT_H (1 << 7)
#define PORT_J (1 << 8)
#define PORT_K (1 << 9)
#define PORT_L (1 << 10)
#define ALL_PORTS (PORT_A|PORT_B|PORT_C|PORT_E|PORT_F|PORT_G|PORT_H|PORT_J|PORT_K|PORT_L)

extern uint32_t port_masks[11];

// Core Definition
typedef struct CoreDef {
	uint8_t type;
	bool default_addrs;			// Set true to use default memory addresses
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
	uint32_t ports;				// The ports available
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

extern const CoreDef default_core;

void setDefaultAddresses(CoreDef *core);

#ifdef __cplusplus
}
#endif

#endif // DEVICES_h
