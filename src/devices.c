// devices.c

#include "devices.h"
#include "opcode_defs.h"

uint32_t port_masks[11] = {
                           PORT_A,
						   PORT_B,
						   PORT_C,
						   PORT_D,
						   PORT_E,
						   PORT_F,
						   PORT_G,
						   PORT_H,
						   PORT_J,
						   PORT_K,
						   PORT_L
						  };

const CoreDef default_core = {
	.type 			= CORE_L_XMEGA,
	.default_addrs 	= true,
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
	.ports			= ALL_PORTS,
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


void setDefaultAddresses(CoreDef *core) {
	core->SP_addr 		= default_core.SP_addr;
	core->EIND_addr		= default_core.EIND_addr;
	core->RAMPZ_addr	= default_core.RAMPZ_addr;
	core->port_a		= default_core.port_a;
	core->port_b		= default_core.port_b;
	core->port_c		= default_core.port_c;
	core->port_d		= default_core.port_d;
	core->port_e		= default_core.port_e;
	core->port_f		= default_core.port_f;
	core->port_g		= default_core.port_g;
	core->port_h		= default_core.port_h;
	core->port_j		= default_core.port_j;
	core->port_k		= default_core.port_k;
	core->port_l		= default_core.port_l;
}
