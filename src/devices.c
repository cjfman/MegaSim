// devices.c

#include <stdlib.h>
#include "devices.h"
#include "opcode_defs.h"

// Default Ports
#define port_a	0x20
#define port_b	0x23
#define port_c	0x26
#define port_d	0x29
#define port_e	0x2C
#define port_f	0x2F
#define port_g	0x32
#define port_h	0x300
#define port_j	0x103
#define port_k	0x106
#define port_l	0x109
#define port_a_map { 0,	1,	 2,	 3,	 4,	 5,	 6,	 7}
#define port_b_map { 8,	9,	10,	11,	12,	13,	14,	15}
#define port_c_map {16, 17,	18,	19,	20,	21,	22,	23}
#define port_d_map {24,	25,	26,	27,	28,	29,	30,	31}
#define port_e_map {32,	33,	34,	35,	36,	37,	38,	39}
#define port_f_map {40,	41,	42,	43,	44,	45,	46,	47}
#define port_g_map {48,	49,	50,	51,	52,	53,	54,	55}
#define port_h_map {56,	57,	58,	59,	60,	61,	62,	63}
#define port_j_map {64,	65,	66,	67,	68,	69,	70,	71}
#define port_k_map {72,	73,	74,	75,	76,	77,	78,	79}
#define port_l_map {80,	81,	82,	83,	84,	85,	86,	87}


// Default timers
// {base address, TIMSKn, TIFRn, OCnA, OCnB, Tn}
#define timer0 { 0x46, 0x6E, 0x35, 1, 2, 3}
#define timer1 { 0x84, 0x6F, 0x36, 0, 0, 0}
#define timer2 { 0xB2, 0x70, 0x37, 0, 0, 0}
#define timer3 { 0x94, 0x71, 0x38, 0, 0, 0}
#define timer4 { 0xA4, 0x72, 0x39, 0, 0, 0}
#define timer5 {0x124, 0x73, 0x3A, 0, 0, 0}

const CoreDef default_core = {
	.type 			= CORE_L_XMEGA,
//	.default_addrs 	= true,
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
	.ports			= {port_a, port_b, port_c, port_d, port_e, port_f,
					  port_g, port_h, port_j, port_k, port_l, 0},
	.port_maps		= {port_a_map, port_b_map, port_c_map, port_d_map, 
					   port_e_map, port_f_map, port_g_map, port_h_map, 
					   port_j_map, port_k_map, port_l_map, 0},
	.timers8bit		= {timer0, timer2, 0},
	.timers16bit	= {timer1, timer3, timer4, timer5, 0}
};


/*
void setDefaultAddresses(CoreDef *core) {
	core->SP_addr 		= default_core.SP_addr;
	core->EIND_addr		= default_core.EIND_addr;
	core->RAMPZ_addr	= default_core.RAMPZ_addr;
}
*/
