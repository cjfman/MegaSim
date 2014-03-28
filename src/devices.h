// MegaTest Devices.h
//
// Developed by Charles Franklnin
//

#ifndef DEVICES_h
#define DEVICES_h

#ifdef __cplusplus
extern "C" {
#endif

// Core Definition
typedef struct CoreDef {
	uint8_t type;
	unsigned int mem_size;		// Size in bytes
	unsigned int io_mem_size;	// Size in bytes
	unsigned int prog_mem_size;	// Size in words
	unsigned int pc_size;		// Size in bits
	uint32_t SP_addr;			// The address of the SP in IO space
	uint32_t EIND_addr;
} CoreDef;

#ifdef __cplusplus
}
#endif

#endif // DEVICES_h
