// devices.c

#include <stdlib.h>
#include "devices.h"
#include "opcode_defs.h"

/*
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
*/

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
#define port_b_map { 8,	9,	10,	11,	12,	13,	13,	14}
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
uint16_t timer0[6] = { 0x46, 0x6E, 0x35, 1, 2, 3};
uint16_t timer1[6] = { 0x84, 0x6F, 0x36, 0, 0, 0};
uint16_t timer2[6] = { 0xB2, 0x70, 0x37, 0, 0, 0};
uint16_t timer3[6] = { 0x94, 0x71, 0x38, 0, 0, 0};
uint16_t timer4[6] = { 0xA4, 0x72, 0x39, 0, 0, 0};
uint16_t timer5[6] = {0x124, 0x73, 0x3A, 0, 0, 0};

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
	.ports			= {port_a, port_b, port_c, port_d, port_e, port_f,
					  port_g, port_h, port_j, port_k, port_l, 0},
	.port_maps		= {port_a_map, port_b_map, port_c_map, port_d_map, 
					   port_e_map, port_f_map, port_g_map, port_h_map, 
					   port_j_map, port_k_map, port_l_map, 0},
	.timers8bit		= {timer0, timer2, NULL},
	.timers16bit	= {timer1, timer3, timer4, timer5, NULL}
	/*
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
	.port_l_map		= {80,	81,	82,	83,	84,	85,	86,	87},
	*/
	/*
	.timer0			= 0x46,
	.timer1			= 0x84,
	.timer2			= 0xB2,
	.timer3			= 0x94,
	.timer4			= 0xA4,
	.timer5			= 0x124,
	.timer0_bits	= 8,
	.timer1_bits	= 16,
	.timer2_bits	= 8,
	.timer3_bits	= 16,
	.timer4_bits	= 16,
	.timer5_bits	= 16,
	.TIMSK0			= 0x6E,
	.TIMSK1			= 0x6F,
	.TIMSK2			= 0x70,
	.TIMSK3			= 0x71,
	.TIMSK4			= 0x72,
	.TIMSK5			= 0x73,
	.TIFR0			= 0x35,
	.TIFR1			= 0x36,
	.TIFR2			= 0x37,
	.TIFR3			= 0x38,
	.TIFR4			= 0x39,
	.TIFR5			= 0x3A,
	.OC0A			= 1,
	.OC0B			= 2,
	.T0				= 3
	*/
};


void setDefaultAddresses(CoreDef *core) {
	core->SP_addr 		= default_core.SP_addr;
	core->EIND_addr		= default_core.EIND_addr;
	core->RAMPZ_addr	= default_core.RAMPZ_addr;
	/*
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
	*/
}

