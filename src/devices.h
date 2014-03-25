// MegaTest Devices.h
//
// Developed by Charles Franklnin
//

#ifndef DEVICES_h
#define DEVICES_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct coreType {
	unsigned int mem_size;		// Size in bytes
	unsigned int prog_mem_size;	// Size in words
	unsigned int pc_size;		// Size in bits
} coreType;

#ifdef __cplusplus
}
#endif

#endif // DEVICES_h
