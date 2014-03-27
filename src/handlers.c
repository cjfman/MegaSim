// handlers.c
//
// Developed by Charles Franklin
//
// MIT Lisense
//

#include "handlers.h"

#error BRx not implemented
#error BREAK not implemented
#error xCALL not implemented
#error xJMP not implemented
#error CPSE not implemented
#error xLPM not implemented
#error IO not implemented

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
	return 0;
}

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
	return 0;
}

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
	return 0;
}

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
	return 0;
}

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
	*ireg->ireg = res;
	return 0;
}

// Logical AND
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
	return;
}

// Logical AND with Immediate
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
	return 0;
}

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
	return 0;
}

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
	return 0;
}

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
	return 0;
}

int COM_run(Instruction *inst) {
	uint8_t res = ~regs[inst->D];
	// Calculate sreg
	sreg[VREG] = 0;
	CALC_N;
	CALC_Z;
	sreg[CREG] = 1;
	CALC_S;
	// Store result
	regs[inst->D] = res;
	return 0;
}

// Two's Complement
int NEG_run(Instruction *inst) {
	uint8_t res = 0 - regs[inst->D];
	// Get bits
	GET_RD3
	GET_RES3;
	// Calculate sreg
	sreg[HREG] = (res3 | rd3);
	sreg[VREG] = (res == 0x80);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (res != 0x00);
	CALC_S;
	// Save result
	regs[inst->D] = res;
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
	return 0;
}

// Is LDI
/*int SER_run(Instruction *inst) {
	regs[inst-D] = 0xFF;
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
	bool bit = (res >> 15) 0x01;
	// Calculate sreg
	sreg[CREG] = bit;
	CALC_Z;
	regps[0] = res;
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
	bool bit = (res >> 15) 0x01;
	// Calculate sreg
	sreg[CREG] = bit;
	CALC_Z;
	regps[0] = res;
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
	bool bit = (res >> 15) 0x01;
	res >>= 1;
	// Calculate sreg
	sreg[CREG] = bit;
	CALC_Z;
	regps[0] = res;
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
	bool bit = (res >> 15) 0x01;
	res >>= 1;
	// Calculate sreg
	sreg[CREG] = bit;
	CALC_Z;
	regps[0] = res;
	return 0;
}

#ifdef XMEGA_SUPPORTED
int DES_run(Instruction *inst);
#endif

////// Branch Instructions ////////////////

// Relative Jump
int RJMP_run(Instruction *inst) {
	pc += inst->A;	// Plus one handled globally
	if (pc + 1 >= prog_mem->size) {
		error_value = pc;
		return PROG_MEM_ERROR;
	}
	return 0;
}

// Indirect Jump
int IJMP_run(Instruction *inst) {
	pc = *RZ;
	if (pc >= prog_mem->size) {
		error_value = pc;
		return PROG_MEM_ERROR;
	}
	pc--; 	// Correct for global increment
	return 0;
}

// Extended Indirect Jump
int EIJMP_run(Instruction *inst) {
	pc = *RZ;
	pc &= 0xFF;
	pc |= *EIND << 8;
	if (pc >= prog_mem->size) {
		error_value = pc;
		return PROG_MEM_ERROR;
	}
	pc--;	// Correct for global increment
	return 0;
}

// Jump
int JMP_run(Instruction *inst) {
	pc = inst->AL;
	if (pc >= prog_mem->size) {
		error_value = pc;
		return PROG_MEM_ERROR;
	}
	pc--;	// Correct for global increment
	return 0;
}

int RCALL_run(Instruction *inst) {
	pc += inst->A; 	// Plus one handled by global increment
	if (pc >= prog_mem->size) {
		error_value = pc;
		return PROG_MEM_ERROR;
	}
	if (core->pc_size == 22) {
		main_mem[(*SP)--] = (pc >> 16) & 0xFF;
	}
	main_mem[(*SP)--] = (pc >> 8) & 0xFF; 
	main_mem[(*SP)--] = pc & 0xFF;
	if (*SP >= core->mem_size) {
		error_value = *SP;
		return SP_ERROR;
	}
	return 0;	
}

int EICALL_run(Instruction *inst) {
	pc = *RZ;
	pc &= 0xFF;
	pc |= *EIND << 8;
	if (pc >= prog_mem->size) {
		error_value = pc;
		return PROG_MEM_ERROR;
	}
	pc--;	// Correct for global increment
	if (core->pc_size == 22) {
		main_mem[(*SP)--] = (pc >> 16) & 0xFF;
	}
	main_mem[(*SP)--] = (pc >> 8) & 0xFF; 
	main_mem[(*SP)--] = pc & 0xFF;
	if (*SP >= core->mem_size) {
		error_value = *SP;
		return SP_ERROR;
	}
	return 0;	
}

int ICALL_run(Instruction *inst) {
}

int CALL_run(Instruction *inst);
int RET_run(Instruction *inst);
int RETI_run(Instruction *inst);
int SBRC_run(Instruction *inst);
int SBRS_run(Instruction *inst);
int SBIC_run(Instruction *inst);
int SBIS_run(Instruction *inst);

int CP_run(Instruction *inst) {
	uint8_t = regs[inst->D] - regs[inst->R];
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_RR3;
	GET_RR7;
	GET_RES7;
	// Calculate sreg
	sreg[HREG] = (!rd3 && rr3) || (rr3 && res3) || (res3 && !rd3);
	sreg[VREG] = (rd7 && !rr7 && !res7) || (!rd7 && rr7 && res7);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (!rd7 && rr7) || (rr7 && res7) || (res7 && !rd7);
	CALC_S;
	return 0;
}

int CPC_run(Instruction *inst) {
	uint8_t = regs[inst->D] - regs[inst->R] - sreg[CREG];
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_RR3;
	GET_RR7;
	GET_RES7;
	// Calculate sreg
	sreg[HREG] = (!rd3 && rr3) || (rr3 && res3) || (res3 && !rd3);
	sreg[VREG] = (rd7 && !rr7 && !res7) || (!rd7 && rr7 && res7);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (!rd7 && rr7) || (rr7 && res7) || (res7 && !rd7);
	CALC_S;
	return 0;
}

int CPI_run(Instruction *inst) {
	uint8_t = regs[inst->D] - regs[inst->K];
	// Get bits
	GET_RD3;
	GET_RD7;
	GET_RR3;
	GET_RR7;
	GET_RES7;
	// Calculate sreg
	sreg[HREG] = (!rd3 && k3) || (k3 && res3) || (res3 && !rd3);
	sreg[VREG] = (rd7 && !k7 && res7) || (!rd7 && k7 && res7);
	CALC_N;
	CALC_Z;
	sreg[CREG] = (!rd7 && k7) || (k7 && res7) || (res7 && rd7);
	CALC_S;
	return 0;
}

int CPSE_run(Instruction *inst);
int BRx_run(Instruction *inst);

////// Data Transfer Instructions//////////

int MOV_run(Instruction *inst) {
	regs[inst->D] = regs[inst->R];
	return 0;
}

int MOVW_run(Instruction *inst) {
	regps[inst->D >> 1] = regps[inst->R >> 1];
	return 0;
}

int LD_run(Instruction *inst);
int LDD_run(Instruction *inst);
int LDI_run(Instruction *inst);
int LDS_run(Instruction *inst);
int LPM_run(Instruction *inst);
int ELPM_run(Instruction *inst);
int SPM_run(Instruction *inst);
int ST_run(Instruction *inst);
int STD_run(Instruction *inst);
int STS_run(Instruction *inst);
int IN_run(Instruction *inst);
int OUT_run(Instruction *inst);
int POP_run(Instruction *inst);
int PUSH_run(Instruction *inst);
int XCH_run(Instruction *inst);
#ifdef XMEGA_SUPPORTED
int LAC_run(Instruction *inst);
int LAS_run(Instruction *inst);
int LAT_run(Instruction *inst);
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
	CALC_S;
	sreg[VREG] = sreg[NREG] ^ sreg[CREG];
	// Save result
	regs[inst->D] = res;
	return 0;
}

// Logical Shift Right
int LSR_run(Instruction *inst) {
	uint8_t res = regs[inst->D] >> 1;
	// Get bits
	GET_RD0;
	GET_RD3;
	GET_RD7;
	GET_RES7;
	// Calculate sreg
	sreg[NREG] = 0;
	CALC_Z;
	sreg[CREG] = rd0;
	CALC_S;
	sreg[VREG] = sreg[NREG] ^ sreg[CREG];
	// Save result
	regs[inst->D] = res;
	return 0;
}

int ROL_run(Instruction *inst);
int ROR_run(Instruction *inst);

// Arithmetic Shift Right
int ASR_run(Instruction *inst) {
	uint8_t res = regs[inst->D] >> 1;	// Do shift
	res |= regs[inst->D] & 0x80;		// Calc sign bit
	// Get bits
	GET_RD0;
	GET_RD7
	// Calc sreg
	CALC_N;
	CALC_Z;
	sreg[CREG] = rd0;
	CALC_S;
	sreg[VREG] = sreg[NREG]^sreg[CREG];
	// Save result
	regs[inst->D] = res;
	return 0;
}

int SWAP_run(Instruction *inst);

int SBI_run(Instruction *inst);

int CBI_run(Instruction *inst) {
	io_mem[inst->A] = ~(0x01 << inst->R);
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
	return 0;
}

int BLD_run(Instruction *inst) {
	bool set = (sreg[TREG] >> 6) & 0x01;
	if (set) {
		bitregs[inst->D] |= 0x01 << inst->R;
	}
	else {
		bitregs[inst->D] &= ~(0x01 << inst->R;);
	}
	return 0;
}

int SEx_run(Instruction *inst);
int CLx_run(Instruction *inst);

////// MCU Control Instructions //////////////

int BREAK_run(Instruction *inst);
int NOP_run(Instruction *inst);
int SLEEP_run(Instruction *inst);
int WDR_run(Instruction *inst);

