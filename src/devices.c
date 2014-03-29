// devices.c

#include "devices.h"
#include "opcode_defs.h"

CoreDef default_core = {
	.type 			= CORE_L_XMEGA,
	.mem_size 		= 65536,
	.prog_mem_size 	= 128*1024/2,
	.sram_start		= 0x0100,
	.sram_end		= 0xFFFF,
	.pc_size 		= 16,
	.sreg_addr		= 0x5F,
	.SP_addr		= 0x5D,
	.EIND_addr		= 0
};
