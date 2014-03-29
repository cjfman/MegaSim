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
} CoreDef;

extern CoreDef default_core;

#ifdef __cplusplus
}
#endif

#endif // DEVICES_h
