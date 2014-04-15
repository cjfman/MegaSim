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
	.EIND_addr		= 0x5C,
	.RAMPZ_addr		= 0x5B,
	.num_pins		= 88,
	.port_a			= 0x20,
	.port_b			= 0x23,
	.port_c			= 0x26,
	.port_d			= 0x29,
	.port_e			= 0x2C,
	.port_f			= 0x2F,
	.port_g			= 0x32,
	.port_h			= 0x300,
	.port_j			= 0x103,
	.port_k			= 0x106,
	.port_l			= 0x109,
	.port_a_map		= { 0,	 1,	 2,	 3,	 4,	 5,	 6,	 7},
	.port_b_map		= { 8,	 9,	10,	11,	12,	13,	13,	14},
	.port_c_map		= {16,  17,	18,	19,	20,	21,	22,	23},
	.port_d_map		= {24,	25,	26,	27,	28,	29,	30,	31},
	.port_e_map		= {32,	33,	34,	35,	36,	37,	38,	39},
	.port_f_map		= {40,	41,	42,	43,	44,	45,	46,	47},
	.port_g_map		= {48,	49,	50,	51,	52,	53,	54,	55},
	.port_h_map		= {56,	57,	58,	59,	60,	61,	62,	63},
	.port_j_map		= {64,	65,	66,	67,	68,	69,	70,	71},
	.port_k_map		= {72,	73,	74,	75,	76,	77,	78,	79},
	.port_l_map		= {80,	81,	82,	83,	84,	85,	86,	87}
};
