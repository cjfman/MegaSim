// handlers.c
//
// Developed by Charles Franklin
//
// MIT Lisense
//

#include <stdio.h>
#include <string.h>
#include "handlers.h"
#include "core.h"
#include "debugterm.h"
#include "opcode_defs.h"

#warning SLEEP/WDR not implemeted
#warning SPM not implemented

#define UNUSED(p) ((p)=(p))

int (*handlers[NUM_CODES])(Instruction*) = {
	ILLOP_run,	// 0
	ADC_run,	// 1
	SUB_run,	// 2
	SUBI_run,	// 3
	SBC_run,	// 4
	SBCI_run,	// 5
	AND_run,	// 6
	ANDI_run,	// 7
	OR_run,		// 8
	ORI_run,	// 9
	EOR_run,	// 10
	COM_run,	// 11
	NEG_run,	// 12
	unhandled_run,	// 13
	unhandled_run,	// 14
	INC_run,	// 15
	DEC_run,	// 16
	unhandled_run,	// 17
	unhandled_run,	// 18
	unhandled_run,	// 19
	RJMP_run,	// 20
	RCALL_run,	// 21
	RET_run,	// 22
	RETI_run,	// 23
	CPSE_run,	// 24
	CP_run,	// 25
	CPC_run,	// 26
	CPI_run,	// 27
	SBRC_run,	// 28
	SBRS_run,	// 29
	SBIC_run,	// 30
	SBIS_run,	// 31
	BRx_run,	// 32
	ADD_run,	// 33
	unhandled_run,	// 34
	unhandled_run,	// 35
	unhandled_run,	// 36
	unhandled_run,	// 37
	unhandled_run,	// 38
	unhandled_run,	// 39
	unhandled_run,	// 40
	unhandled_run,	// 41
	unhandled_run,	// 42
	unhandled_run,	// 43
	unhandled_run,	// 44
	unhandled_run,	// 45
	unhandled_run,	// 46
	unhandled_run,	// 47
	unhandled_run,	// 48
	unhandled_run,	// 49
	unhandled_run,	// 50
	unhandled_run,	// 51
	LD_run,	// 52
	ST_run,	// 53
	MOV_run,	// 54
	LDI_run,	// 55
	IN_run,	// 56
	OUT_run,	// 57
	LPM_run,	// 58
	SBI_run,	// 59
	CBI_run,	// 60
	LSL_run,	// 61
	LSR_run,	// 62
	ROL_run,	// 63
	ROR_run,	// 64
	ASR_run,	// 65
	SWAP_run,	// 66
	unhandled_run,	// 67
	unhandled_run,	// 68
	BST_run,	// 69
	BLD_run,	// 70
	SEx_run,	// 71
	CLx_run,	// 72
	NOP_run,	// 73
	SLEEP_run,	// 74
	WDR_run,	// 75
	LD2_run,	// 76
	LD3_run,	// 77
	ST2_run,	// 78
	ST3_run,	// 79
	unhandled_run,	// 80
	unhandled_run,	// 81
	unhandled_run,	// 82
	unhandled_run,	// 83
	unhandled_run,	// 84
	unhandled_run,	// 85
	unhandled_run,	// 86
	unhandled_run,	// 87
	unhandled_run,	// 88
	unhandled_run,	// 89
	ADIW_run,	// 90
	SBIW_run,	// 91
	IJMP_run,	// 92
	ICALL_run,	// 93
	LDD_run,	// 94
	LDS_run,	// 95
	STD_run,	// 96
	STS_run,	// 97
	PUSH_run,	// 98
	POP_run,	// 99
	JMP_run,	// 100
	CALL_run,	// 101
	ELPM_run,	// 102
	MUL_run,	// 103
	MULS_run,	// 104
	MULSU_run,	// 105
	FMUL_run,	// 106
	FMULS_run,	// 107
	FMULSU_run,	// 108
	MOVW_run,	// 109
	LPM_run,	// 110
	SPM_run,	// 111
	BREAK_run,	// 112
	EIJMP_run,	// 113
	EICALL_run,	// 114
#ifdef XMEGA_SUPPORTED
	DES_run,	// 115
	XCH_run,	// 116
	LAS_run,	// 117
	LAC_run,	// 118
	LAT_run,	// 119
#else
	unhandled_run,	// 115
	unhandled_run,	// 116
	unhandled_run,	// 117
	unhandled_run,	// 118
	unhandled_run,	// 119
#endif
	unhandled_run,	// 120
	unhandled_run,	// 121
	unhandled_run,	// 122
	unhandled_run,	// 123
	unhandled_run,	// 124
	unhandled_run,	// 125
	unhandled_run,	// 126
	unhandled_run	// 127
};

// Errors /////////////////////////////////////////////////////////////

void printError(int error) {
	char const *c;
	switch(error) {
	case MEM_ERROR:
		c = "Memory Error";
		break;
	case PROG_MEM_ERROR:
		c = "Prgram Memroy Error";
		break;
	case SP_ERROR:
		c = "Stack Pointer Error";
		break;
	case BAD_REG_ERROR:
		c = "Bad Register Error";
		break;
	case ILLOP_ERROR:
		c = "ILLOP Error";
		break;
	case UNHANDLED_ERROR:
		c ="Unhandled Instruction Error";
		break;
	case PC_ERROR:
		c = "Program Counter Error";
		break;
	case EXIT_ERROR:
		c = "Program Quiting";
		break;
	default:
		printf("Code %d: ", error);
		c = "Unknown Error";
		break;
	}
	printf("%s at PC = 0x%X\n", c, pc);
}

/////////////////////////////////
// Instruction Implementations
/////////////////////////////////

// Macros for getting bits
#define GET_RD0 bool rd0 = regs[inst->D] & 0x01;
#define GET_RD3 bool rd3 = (regs[inst->D] >> 3) & 0x01;
#define GET_RD7 bool rd7 = (regs[inst->D] >> 7) & 0x01;
#define GET_RR3 bool rr3 = (regs[inst->R] >> 3) & 0x01;
#define GET_RR7 bool rr7 = (regs[inst->R] >> 7) & 0x01;
#define GET_K0 bool k0 = regs[inst->K] & 0x01;
#define GET_K3 bool k3 = (regs[inst->K] >> 3) & 0x01;
#define GET_K7 bool k7 = (regs[inst->K] >> 7) & 0x01;
#define GET_RES3 bool res3 = (res >> 3) & 0x01;
#define GET_RES7 bool res7 = (res >> 7) & 0x01;
#define GET_RES15 bool res15 = (res >> 15) & 0x01;

// Bits for 16-bit operands
#define GET_RDH7 bool rdh7 = (dword >> 15) & 0x01;

// Macros for calculating sreg bits
#define CALC_N sreg[NREG] = res7;
#define CALC_Z sreg[ZREG] = (res == 0);
#define CALC_S sreg[SREG] = (sreg[NREG]^sreg[VREG]) & 0x01;

void reset_run(void) {
	memset(main_mem, 0, coredef->sram_start);
	pc = 0;
	cycle_count = 0;
	*SP = coredef->sram_end;
}

int unhandled_run(Instruction *inst) {
	error_val = inst->op;
	return UNHANDLED_ERROR;
}

int ILLOP_run(Instruction *inst) {
	error_val = *inst->ireg;
	return ILLOP_ERROR;
}

////// Arithmetic and Logic Instructions ////////////////

int ADD_run(Instruction *inst) {
	uint8_t res = regs[inst->D] + regs[inst->R]; 
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_RR3;
	GET_RR7;
	GET_RES3;
	GET_RES7;
	// Calc sreg bits
	sreg[HREG] = (rd3 && rr3) || (rr3 && !res3) || (!res3 && rd3);
	sreg[VREG] = (rd7 && rr7 && !res7) || (!rd7 && !rr7 && res7);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (rd7 && rr7) || (rr7 && !res7) || (!res7 && rd7);
	CALC_S;
	// Save Result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Add with Carry
int ADC_run(Instruction *inst) {
	uint8_t res = regs[inst->D] + regs[inst->R] + sreg[CREG];
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_RR3;
	GET_RR7;
	GET_RES3;
	GET_RES7;
	// Calc sreg bits
	sreg[HREG] = (rd3 && rr3) || (rr3 && !res3) || (!res3 && rd3);
	sreg[VREG] = (rd7 && rr7 && !res7) || (!rd7 && !rr7 && res7);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (rd7 && rr7) || (rr7 && !res7) || (!res7 && rd7);
	CALC_S;
	// Save Result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}
	
// Add Immediate to Word
int ADIW_run(Instruction *inst) {
	uint16_t dword = *inst->ireg;
	uint16_t res = dword + inst->K;
	// Get bits
	GET_RES15;
	GET_RDH7;
	// Calculate sreg bits
	sreg[VREG] = res15 && !rdh7;
	sreg[NREG] = res15;
	CALC_Z;
	sreg[CREG] = (!res15 && rdh7);
	CALC_S;
	// Save Result
	*inst->ireg = res;
	pc += inst->wsize;
	return 0;
}

// Subtract
int SUB_run(Instruction *inst) {
	uint8_t res = regs[inst->D] - regs[inst->R];
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_RR3;
	GET_RR7;
	GET_RES3;
	GET_RES7;
	// Calculate sreg
	sreg[HREG] = (!rd3 && rr3) || (rr3 && res3) || (res3 && !rd3);
	sreg[VREG] = (rd7 && !rr7 && !res7) || (!rd7 && rr7 && res7);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (!rd7 && rr7) || (rr7 && res7) || (res7 && !rd7);
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Subtract Immediate
int SUBI_run(Instruction *inst) {
	uint8_t res = regs[inst->D] - inst->K;
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_K3;
	GET_K7;
	GET_RES3;
	GET_RES7;
	// Calc sreg
	sreg[HREG] = (!rd3 && k3) || (k3 && res3) || (res3 && !rd3);
	sreg[VREG] = (rd7 && !k7 && res7) || (!rd7 && k7 && res7);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (!rd7 && k7) || (k7 && res7) || (res7 && rd7);
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Subtract with Carry
int SBC_run(Instruction *inst) {
	uint8_t res = regs[inst->D] - regs[inst->R] - sreg[CREG];
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_RR3;
	GET_RR7;
	GET_RES3;
	GET_RES7;
	// Calculate sreg
	sreg[HREG] = (!rd3 && rr3) || (rr3 && res3) || (res3 && !rd3);
	sreg[VREG] = (rd7 && !rr7 && !res7) || (!rd7 && rr7 && res7);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (!rd7 && rr7) || (rr7 && res7) || (res7 && !rd7);
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Subtract with Carry Immediate
int SBCI_run(Instruction *inst) {
	uint8_t res = regs[inst->D] - inst->K - sreg[CREG];
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_K3;
	GET_K7;
	GET_RES3;
	GET_RES7;
	// Calc sreg
	sreg[HREG] = (!rd3 && k3) || (k3 && res3) || (res3 && !rd3);
	sreg[VREG] = (rd7 && !k7 && res7) || (!rd7 && k7 && res7);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (!rd7 && k7) || (k7 && res7) || (res7 && rd7);
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Subtract Immediate from Word
int SBIW_run(Instruction *inst) {
	uint16_t dword = *inst->ireg;
	uint16_t res = dword - inst->K;
	// Get bits
	GET_RDH7;
	GET_RES15;
	// Calculate sreg
	sreg[VREG] = (rdh7 && !res15);
	sreg[NREG] = res15;
	CALC_Z;
	sreg[CREG] = (res15 && !rdh7);
	CALC_S;
	// Save result
	*inst->ireg = res;
	pc += inst->wsize;
	return 0;
}

// Bitwise AND
int AND_run(Instruction *inst) {
	uint8_t res = regs[inst->D] & regs[inst->R];
	// Get bits
	GET_RES7;
	// Calc sreg
	sreg[VREG] = 0;
	CALC_N;
	CALC_Z;
	CALC_S;
	// Save Result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Bitwise AND with Immediate
int ANDI_run(Instruction *inst) {
	uint8_t res = regs[inst->D] & (uint8_t)inst->K;
	// Get bits
	GET_RES7;
	// Calc sreg
	sreg[VREG] = 0;
	CALC_N;
	CALC_Z;
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Bitwise OR
int OR_run(Instruction *inst) {
	uint8_t res = regs[inst->D] | regs[inst->R];
	// Get bits
	GET_RES7;
	// Calculate sreg
	sreg[VREG] = 0;
	CALC_N;
	CALC_Z;
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Bitwise OR with Immediate
int ORI_run(Instruction *inst) {
	uint8_t res = regs[inst->D] | (uint8_t)inst->K;
	// Get bits
	GET_RES7;
	// Calc sreg
	sreg[VREG] = 0;
	CALC_N;
	CALC_Z;
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Bitwise XOR
int EOR_run(Instruction *inst) {
	uint8_t res = regs[inst->D] ^ regs[inst->R];
	// Get bits
	GET_RES7;
	// Calculate sreg
	sreg[VREG] = 0;
	CALC_N;
	CALC_Z;
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// One's Complement
int COM_run(Instruction *inst) {
	uint8_t res = ~regs[inst->D];
	// GET BITS
	GET_RES7
	// Calculate sreg
	sreg[VREG] = 0;
	CALC_N;
	CALC_Z;
	sreg[CREG] = 1;
	CALC_S;
	// Store result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Two's Complement
int NEG_run(Instruction *inst) {
	uint8_t res = 0 - regs[inst->D];
	// Get bits
	GET_RD3
	GET_RES3;
	GET_RES7;
	// Calculate sreg
	sreg[HREG] = (res3 | rd3);
	sreg[VREG] = (res == 0x80);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (res != 0x00);
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Increment
int INC_run(Instruction *inst) {
	uint8_t res = ++regs[inst->D];
	// Get bits
	GET_RES7;
	// Calculate sreg
	sreg[VREG] = (res == 0x7F);
	CALC_N;
	CALC_Z;
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Decrement
int DEC_run(Instruction *inst) {
	uint8_t res = regs[inst->D] - 1;
	// Get bits
	GET_RES7;
	// Calculate sreg
	sreg[VREG] = (res == 0x7F);
	CALC_N;
	CALC_Z;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Is LDI
/*int SER_run(Instruction *inst) {
	regs[inst-D] = 0xFF;
	pc += inst->wsize;
	return 0;
}*/

// Multiply Unsigned
int MUL_run(Instruction *inst) {
	uint16_t res = regs[inst->D] * regs[inst->R];
	bool bit = res >> 15;	// Get res15
	// Calculate sreg
	sreg[CREG] = bit;
	CALC_Z;
	// Store result
	regps[0] = res;
	pc += inst->wsize;
	return 0;
}

// Multiply Signed
int MULS_run(Instruction *inst) {
	int16_t opL, opR;	// Left and right operands
#ifdef IS_ARITHMETIC_RS
	opL = ((int16_t)regs[inst->D] << 8) >> 8;
	opR = ((int16_t)regs[inst->R] << 8) >> 8;
#else
	opL = (int16_t)regs[inst->D];
	opR = (int16_t)regs[inst->D];
	if (opL & 0x0080) {
		opL |= 0xFF00;
	}
	if (opR & 0x0080) {
		opR |= 0xFF00;
	}
#endif
	int16_t res = opL * opR;
	bool bit = (res >> 15) & 0x01;
	// Calculate sreg
	sreg[CREG] = bit;
	CALC_Z;
	regps[0] = res;
	pc += inst->wsize;
	return 0;
}

// Multiply Signed with Unsigned
int MULSU_run(Instruction *inst) {
	int16_t opL; 	// Left operand Rd is signed.
	uint16_t opR;	// Right operand Rr is signed.
	opR = (uint16_t)regs[inst->R];
#ifdef IS_ARITHMETIC_RS
	opL = ((int8_t)regs[inst->D] << 8) >> 8;
#else
	opL = (int16_t)regs[inst->D];
	if (opL & 0x0080) {
		opL |= 0xFF00;
	}
#endif
	int16_t res = opL * opR;
	bool bit = (res >> 15)  & 0x01;
	// Calculate sreg
	sreg[CREG] = bit;
	CALC_Z;
	regps[0] = res;
	pc += inst->wsize;
	return 0;
}

// Fractional Multiply Unsigned
int FMUL_run(Instruction *inst) {
	uint16_t res = regs[inst->D] * regs[inst->R];
	bool bit = res >> 15;	// Get res15 before the shift
	res = res << 1;			// DO shift
	// Calculate sreg
	sreg[CREG] = bit;
	CALC_Z;
	// Store result
	regps[0] = res;
	pc += inst->wsize;
	return 0;
}

// Fractional Multiply Signed
int FMULS_run(Instruction *inst) {
	int16_t opL, opR;	// Left and right operands
#ifdef IS_ARITHMETIC_RS
	opL = ((int16_t)regs[inst->D] << 8) >> 8;
	opR = ((int16_t)regs[inst->R] << 8) >> 8;
#else
	opL = (int16_t)regs[inst->D];
	opR = (int16_t)regs[inst->D];
	if (opL & 0x0080) {
		opL |= 0xFF00;
	}
	if (opR & 0x0080) {
		opR |= 0xFF00;
	}
#endif
	int16_t res = opL * opR;
	bool bit = (res >> 15) & 0x01;
	res >>= 1;
	// Calculate sreg
	sreg[CREG] = bit;
	CALC_Z;
	regps[0] = res;
	pc += inst->wsize;
	return 0;
}

// Fractional Multiply Signed with Unsigned
int FMULSU_run(Instruction *inst) {
	int16_t opL; 	// Left operand Rd is signed.
	uint16_t opR;	// Right operand Rr is signed.
	opR = (uint16_t)regs[inst->R];
#ifdef IS_ARITHMETIC_RS
	opL = ((int8_t)regs[inst->D] << 8) >> 8;
#else
	opL = (int16_t)regs[inst->D];
	if (opL & 0x0080) {
		opL |= 0xFF00;
	}
#endif
	int16_t res = opL * opR;
	bool bit = (res >> 15) & 0x01;
	res >>= 1;
	// Calculate sreg
	sreg[CREG] = bit;
	CALC_Z;
	regps[0] = res;
	pc += inst->wsize;
	return 0;
}

#ifdef XMEGA_SUPPORTED
int DES_run(Instruction *inst);
#endif

////// Branch Instructions ////////////////

// Relative Jump
int RJMP_run(Instruction *inst) {
	pc += inst->A + 1;
	if (pc + 1 >= program->size) {
		error_val = pc;
		return PROG_MEM_ERROR;
	}
	return 0;
}

// Indirect Jump
int IJMP_run(Instruction *inst) {
	UNUSED(inst);
	pc = *RZ;
	if (pc >= program->size) {
		error_val = pc;
		return PROG_MEM_ERROR;
	}
	return 0;
}

// Extended Indirect Jump
int EIJMP_run(Instruction *inst) {
	UNUSED(inst);
	pc = *RZ;
	pc &= 0xFF;
	pc |= *EIND << 8;
	if (pc >= program->size) {
		error_val = pc;
		return PROG_MEM_ERROR;
	}
	return 0;
}

// Jump
int JMP_run(Instruction *inst) {
	pc = inst->AL;
	if (pc >= program->size) {
		error_val = pc;
		return PROG_MEM_ERROR;
	}
	return 0;
}

int RCALL_run(Instruction *inst) {
	// Push old PC+1 to stack
	pc++;
	if (coredef->pc_size == 22) {
		//main_mem[(*SP)--] = (pc >> 16) & 0xFF;
		*((uint32_t*)&main_mem[*SP - 2]) &= 0xFF000000;
		*((uint32_t*)&main_mem[*SP - 2]) |= pc & 0x3FFFFF;
		*SP -= 3;
	}
	else {
		*((uint16_t*)&main_mem[*SP - 1]) = pc & 0xFFFF;
		*SP -= 2;
	}
	/*if (coredef->pc_size == 22) {
		main_mem[(*SP)--] = (pc >> 16) & 0xFF;
	}
	main_mem[(*SP)--] = (pc >> 8) & 0xFF; 
	main_mem[(*SP)--] = pc & 0xFF;
	*/
	if (*SP >= coredef->mem_size) {
		error_val = *SP;
		return SP_ERROR;
	}
	// Calculate new PC
	pc += inst->A;
	if (pc >= program->size) {
		error_val = pc;
		return PROG_MEM_ERROR;
	}
	return 0;	
}

int EICALL_run(Instruction *inst) {
	UNUSED(inst);
	// Push old PC+1 to stack
	pc++;
	if (coredef->pc_size == 22) {
		//main_mem[(*SP)--] = (pc >> 16) & 0xFF;
		*((uint32_t*)&main_mem[*SP - 2]) &= 0xFF000000;
		*((uint32_t*)&main_mem[*SP - 2]) |= pc & 0x3FFFFF;
		*SP -= 3;
	}
	else {
		*((uint16_t*)&main_mem[*SP - 1]) = pc & 0xFFFF;
		*SP -= 2;
	}
	/*if (coredef->pc_size == 22) {
		main_mem[(*SP)--] = (pc >> 16) & 0xFF;
	}
	main_mem[(*SP)--] = (pc >> 8) & 0xFF; 
	main_mem[(*SP)--] = pc & 0xFF;
	*/
	if (*SP >= coredef->mem_size) {
		error_val = *SP;
		return SP_ERROR;
	}
	// Calculate new PC
	pc = *RZ;
	pc |= *EIND << 8;
	if (pc >= program->size) {
		error_val = pc;
		return PROG_MEM_ERROR;
	}
	return 0;	
}

int ICALL_run(Instruction *inst) {
	UNUSED(inst);
	// Push old PC+1 to stack
	pc++;
	if (coredef->pc_size == 22) {
		//main_mem[(*SP)--] = (pc >> 16) & 0xFF;
		*((uint32_t*)&main_mem[*SP - 2]) &= 0xFF000000;
		*((uint32_t*)&main_mem[*SP - 2]) |= pc & 0x3FFFFF;
		*SP -= 3;
	}
	else {
		*((uint16_t*)&main_mem[*SP - 1]) = pc & 0xFFFF;
		*SP -= 2;
	}
	/*if (coredef->pc_size == 22) {
		main_mem[(*SP)--] = (pc >> 16) & 0xFF;
	}
	main_mem[(*SP)--] = (pc >> 8) & 0xFF; 
	main_mem[(*SP)--] = pc & 0xFF;
	*/
	if (*SP >= coredef->mem_size) {
		error_val = *SP;
		return SP_ERROR;
	}
	// Calculate new PC
	pc = *RZ;
	if (pc >= program->size) {
		error_val = pc;
		return PROG_MEM_ERROR;
	}
	return 0;
}

int CALL_run(Instruction *inst) {
	// Push old PC+1 to stack
	pc += 2;
	if (coredef->pc_size == 22) {
		//main_mem[(*SP)--] = (pc >> 16) & 0xFF;
		*((uint32_t*)&main_mem[*SP - 2]) &= 0xFF000000;
		*((uint32_t*)&main_mem[*SP - 2]) |= pc & 0x3FFFFF;
		*SP -= 3;
	}
	else {
		*((uint16_t*)&main_mem[*SP - 1]) = pc & 0xFFFF;
		*SP -= 2;
	}
	//main_mem[(*SP)--] = (pc >> 8) & 0xFF; 
	//main_mem[(*SP)--] = pc & 0xFF;
	if (*SP >= coredef->mem_size) {
		error_val = *SP;
		return SP_ERROR;
	}
	// Calculate new PC
	pc = inst->AL;
	if (pc >= program->size) {
		error_val = pc;
		return PROG_MEM_ERROR;
	}
	return 0;
}

int RET_run(Instruction *inst) {
	UNUSED(inst);
	//pc = 0;
	//pc |= main_mem[++(*SP)];
	//pc |= main_mem[++(*SP)] << 8;
	if (coredef->pc_size == 22) {
		pc = *((uint32_t*)&main_mem[*SP + 1]) & 0x3FFFFF;
		*SP += 3;
	}
	else {
		pc = *((uint16_t*)&main_mem[*SP + 1]) & 0xFFF;
		*SP += 2;
	}
	if (*SP >= coredef->mem_size) {
		error_val = *SP;
		return SP_ERROR;
	}
	return 0;
}

int RETI_run(Instruction *inst) {
	UNUSED(inst);
	pc = 0;
	pc |= main_mem[++(*SP)];
	pc |= main_mem[++(*SP)] << 8;
	if (coredef->pc_size == 22) {
		pc = *((uint32_t*)&main_mem[*SP + 1]) & 0x3FFFFF;
		*SP += 3;
	}
	else {
		pc = *((uint16_t*)&main_mem[*SP + 1]) & 0xFFF;
		*SP += 2;
	}
	if (*SP >= coredef->mem_size) {
		error_val = *SP;
		return SP_ERROR;
	}
	sreg[IREG] = true;
	return 0;
}

int SBRC_run(Instruction *inst) {
	uint8_t set = (regs[inst->D] >> inst->R) & 0x01;
	if (!set) {
		int words = (inst + 1)->wsize;
		pc += words;
		inst->cycles = 1 + words;
	}
	else {
		inst->cycles = 1;
	}
	pc += inst->wsize;
	return 0;
}

int SBRS_run(Instruction *inst) {
	uint8_t set = (regs[inst->D] >> inst->R) & 0x01;
	if (set) {
		int words = (inst + 1)->wsize;
		pc += words;
		inst->cycles = 1 + words;
	}
	else {
		inst->cycles = 1;
	}
	pc += inst->wsize;
	return 0;
}

int SBIC_run(Instruction *inst) {
	uint8_t set = (io_mem[inst->A] >> inst->R) & 0x01;
	if (!set) {
		int words = (inst + 1)->wsize;
		pc += words;
		inst->cycles = 1 + words;
	}
	else {
		inst->cycles = 1;
	}
	pc += inst->wsize;
	return 0;
}

int SBIS_run(Instruction *inst) {
	uint8_t set = (io_mem[inst->A] >> inst->R) & 0x01;
	if (set) {
		int words = (inst + 1)->wsize;
		pc += words;
		inst->cycles = 1 + words;
	}
	else {
		inst->cycles = 1;
	}
	pc += inst->wsize;
	return 0;
}

int CP_run(Instruction *inst) {
	uint8_t res = regs[inst->D] - regs[inst->R];
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_RR3;
	GET_RR7;
	GET_RES3;
	GET_RES7;
	// Calculate sreg
	sreg[HREG] = (!rd3 && rr3) || (rr3 && res3) || (res3 && !rd3);
	sreg[VREG] = (rd7 && !rr7 && !res7) || (!rd7 && rr7 && res7);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (!rd7 && rr7) || (rr7 && res7) || (res7 && !rd7);
	CALC_S;
	pc += inst->wsize;
	return 0;
}

int CPC_run(Instruction *inst) {
	uint8_t res = regs[inst->D] - regs[inst->R] - sreg[CREG];
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_RR3;
	GET_RR7;
	GET_RES3;
	GET_RES7;
	// Calculate sreg
	sreg[HREG] = (!rd3 && rr3) || (rr3 && res3) || (res3 && !rd3);
	sreg[VREG] = (rd7 && !rr7 && !res7) || (!rd7 && rr7 && res7);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (!rd7 && rr7) || (rr7 && res7) || (res7 && !rd7);
	CALC_S;
	pc += inst->wsize;
	return 0;
}

int CPI_run(Instruction *inst) {
	uint8_t res = regs[inst->D] - regs[inst->K];
	// Get bits
	GET_RD3;
	GET_RD7;
	//GET_RR3;
	//GET_RR7;
	GET_RES3;
	GET_RES7;
	GET_K3;
	GET_K7;
	// Calculate sreg
	sreg[HREG] = (!rd3 && k3) || (k3 && res3) || (res3 && !rd3);
	sreg[VREG] = (rd7 && !k7 && res7) || (!rd7 && k7 && res7);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (!rd7 && k7) || (k7 && res7) || (res7 && rd7);
	CALC_S;
	pc += inst->wsize;
	return 0;
}

int CPSE_run(Instruction *inst) {
	if (regs[inst->D] == regs[inst->R]) {
		int words = (inst + 1)->wsize;
		pc += words;
		inst->cycles = 1 + words;
	}
	else {
		inst->cycles = 1;
	}
	pc += inst->wsize;
	return 0;
}

int BRx_run(Instruction *inst) {
	// Brach dependent on status register
	// inst->D determins if to branch when set
	// or cleared
	uint8_t branch = (sreg[inst->R] && inst->D) 
		|| !(sreg[inst->R] || inst->D);
	if (branch) {
		pc += inst->K + inst->wsize;
		inst->cycles = 2;
	}
	else {
		inst->cycles = 1;
	}
	pc += inst->wsize;
	return 0;
}

////// Data Transfer Instructions//////////

int MOV_run(Instruction *inst) {
	regs[inst->D] = regs[inst->R];
	pc += inst->wsize;
	return 0;
}

int MOVW_run(Instruction *inst) {
	regps[inst->D >> 1] = regps[inst->R >> 1];
	pc += inst->wsize;
	return 0;
}

int LD_run(Instruction *inst) {
	regs[inst->D] = readMem(*RX);
	pc += inst->wsize;
	return 0;
}

int LD2_run(Instruction *inst) {
	regs[inst->D] = readMem((*inst->ireg)++);
	pc += inst->wsize;
	return 0;
}

int LD3_run(Instruction *inst) {
	regs[inst->D] = readMem(--(*inst->ireg));
	pc += inst->wsize;
	return 0;
}

int LDD_run(Instruction *inst) {
	regs[inst->D] = readMem(*inst->ireg + inst->K);
	pc += inst->wsize;
	return 0;
}

int LDI_run(Instruction *inst) {
	regs[inst->D] = inst->K;
	pc += inst->wsize;
	return 0;
}

int LDS_run(Instruction *inst) {
	regs[inst->D] = readMem(inst->AL);
	pc += inst->wsize;
	return 0;
}

int LPM_run(Instruction *inst) {
	switch(inst->mode) {
	case 1:
		regs[0] = readMem(*RZ);
		break;
	case 2:
		regs[inst->D] = ((uint8_t*)program->data)[*RZ];
		break;
	case 3:
		regs[inst->D] = ((uint8_t*)program->data)[(*RZ)++];
		break;
	}
	pc += inst->wsize;
	return 0;
}

int ELPM_run(Instruction *inst) {
	uint32_t ea = *RZ | (*RAMPZ << 16);
	if (ea >= 2*program->size) {
		return PROG_MEM_ERROR;
	}
	switch(inst->mode) {
	case 1:
		regs[0] = ((uint8_t*)program->data)[ea];
		break;
	case 3:
		(*RZ)++;
	case 2:
		regs[inst->D] = ((uint8_t*)program->data)[ea];
		break;
	}
	pc += inst->wsize;
	return 0;
}

int SPM_run(Instruction *inst) {
	error_val = inst->op;
	return UNHANDLED_ERROR;
}

int ST_run(Instruction *inst) {
	writeMem(*RX, regs[inst->D]);
	pc += inst->wsize;
	return 0;
}

int ST2_run(Instruction *inst) {
	writeMem((*inst->ireg)++, regs[inst->D]);
	pc += inst->wsize;
	return 0;
}

int ST3_run(Instruction *inst) {
	writeMem(--(*inst->ireg), regs[inst->D]);
	pc += inst->wsize;
	return 0;
}

int STD_run(Instruction *inst) {
	writeMem(*inst->ireg + inst->K, regs[inst->D]);
	pc += inst->wsize;
	return 0;
}

int STS_run(Instruction *inst) {
	writeMem(inst->AL, regs[inst->D]);
	pc += inst->wsize;
	return 0;
}

int IN_run(Instruction *inst) {
	regs[inst->D] = readMem(inst->A);
	pc += inst->wsize;
	return 0;
}

int OUT_run(Instruction *inst) {
	writeMem(inst->A, regs[inst->D]);
	pc += inst->wsize;
	return 0;
}

int POP_run(Instruction *inst) {
	regs[inst->D] = main_mem[++(*SP)];
	pc += inst->wsize;
	return 0;
}

int PUSH_run(Instruction *inst) {
	main_mem[(*SP)--] = regs[inst->D];
	pc += inst->wsize;
	return 0;
}

#ifdef XMEGA_SUPPORTED
int XCH_run(Instruction *inst) {
	error_val = XCH;
	return UNHANDLED_ERROR;
}

int LAC_run(Instruction *inst) {
	error_val = LAC;
	return UNHANDLED_ERROR;
}

int LAS_run(Instruction *inst) {
	error_val = LAS;
	return UNHANDLED_ERROR;
}

int LAT_run(Instruction *inst) {
	error_val = LAT;
	return UNHANDLED_ERROR;
}
#endif

////// Bitwise Instructions ///////////////

// Logical Shift Left
int LSL_run(Instruction *inst) {
	uint8_t res = regs[inst->D] << 1;
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_RES7;
	// Calculate sreg
	sreg[HREG] = rd3;
	CALC_N;
	CALC_Z;
	sreg[CREG] = rd7;
	sreg[VREG] = sreg[NREG] ^ sreg[CREG];
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Logical Shift Right
int LSR_run(Instruction *inst) {
	uint8_t res = regs[inst->D] >> 1;
	// Get bits
	GET_RD0;
	//GET_RD3;
	//GET_RD7;
	//GET_RES7;
	// Calculate sreg
	sreg[NREG] = 0;
	CALC_Z;
	sreg[CREG] = rd0;
	sreg[VREG] = sreg[NREG] ^ sreg[CREG];
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Rotate Left Through Carry
int ROL_run(Instruction *inst) {
	uint8_t res = (regs[inst->D] << 1) | sreg[CREG];
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_RES7;
	// Calculate sreg
	sreg[HREG] = rd3;
	CALC_N;
	CALC_Z;
	sreg[CREG] = rd7;
	sreg[VREG] = sreg[NREG] ^ sreg[CREG];
	CALC_S;
	pc += inst->wsize;
	return 0;
}

int ROR_run(Instruction *inst) {
	uint8_t res = (sreg[CREG] << 7) | (regs[inst->D] >> 1);
	// Get Bits
	GET_RES7;
	GET_RD0;
	// Calc sreg
	sreg[CREG] = rd0;
	CALC_Z;
	CALC_N;
	sreg[VREG] = sreg[NREG] ^ sreg[CREG];
	CALC_S;
	// Save res
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

// Arithmetic Shift Right
int ASR_run(Instruction *inst) {
	uint8_t res = regs[inst->D] >> 1;	// Do shift
	res |= regs[inst->D] & 0x80;		// Calc sign bit
	// Get bits
	GET_RD0;
	//GET_RD7;
	GET_RES7;
	// Calc sreg
	CALC_N;
	CALC_Z;
	sreg[CREG] = rd0;
	sreg[VREG] = sreg[NREG]^sreg[CREG];
	CALC_S;
	// Save result
	regs[inst->D] = res;
	pc += inst->wsize;
	return 0;
}

int SWAP_run(Instruction *inst) {
	uint8_t swap = regs[inst->D] & 0x0F;
	regs[inst->D] >>= 4;
	regs[inst->D] |= swap << 4;
	pc += inst->wsize;
	return 0;
}

int SBI_run(Instruction *inst) {
	io_mem[inst->A] |= (0x01 << inst->R);
	pc += inst->wsize;
	return 0;
}

int CBI_run(Instruction *inst) {
	io_mem[inst->A] &= ~(0x01 << inst->R);
	pc += inst->wsize;
	return 0;
}

int BST_run(Instruction *inst) {
	bool set = (regs[inst->D] >> inst->R) & 0x01;
	if (set) { 
		sreg[TREG] = 1;
	}
	else {
		sreg[TREG] = 0;
	}
	pc += inst->wsize;
	return 0;
}

int BLD_run(Instruction *inst) {
	bool set = (sreg[TREG] >> 6) & 0x01;
	if (set) {
		regs[inst->D] |= 0x01 << inst->R;
	}
	else {
		regs[inst->D] &= ~(0x01 << inst->R);
	}
	pc += inst->wsize;
	return 0;
}

int SEx_run(Instruction *inst) {
	sreg[inst->R] = true;
	pc += inst->wsize;
	return 0;
}

int CLx_run(Instruction *inst) {
	sreg[inst->R] = false;
	pc += inst->wsize;
	return 0;
}

////// MCU Control Instructions //////////////

int BREAK_run(Instruction *inst) {
	pc += inst->wsize;
	if (debug_mode) {
		return 0;	// Do not recurse
	}
	return runDebugTerm();
}

int NOP_run(Instruction *inst) {
	pc += inst->wsize;
	return 0;
}

int SLEEP_run(Instruction *inst) {
	UNUSED(inst);
	error_val = SLEEP;
	return UNHANDLED_ERROR;
}

int WDR_run(Instruction *inst) {
	UNUSED(inst);
	error_val = WDR;
	return UNHANDLED_ERROR;
}

