// core.c
//
// Developed by Charles Franklin
//
// MIT Lisense
//

#include <core.h>

#error BRx not implemented
#error BREAK not implemented
#error xCALL not implemented
#error xJMP not implemented
#error CPSE not implemented
#error xLPM not implemented
#error IO not implemented

void decodedInstruction(Instruction *inst, uint16_t opcode) {
	// Most variables are always in the same place if used
	// Get them now before conditional statements
	inst->D = (opcode >> 4) & 0x1F;
	inst->R = (opcode & 0x0F) | ((opcode >> 5) & 0x10);
	inst->K = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
	// Other defaults
	inst->wsize = 1;
	// Get commonly used opcode bit vectors
	unsigned int top9 = opcode >> 9;	
	unsigned int top8 = opcode >> 8;
	unsigned int top7 = opcode >> 7;
	unsigned int top6 = opcode >> 6;
	unsigned int top5 = opcode >> 5;
	unsigned int top4 = opcode >> 4;
	unsigned int low8 = opcode & 0xFF;
	unsigned int low4 = opcode & 0x0F;
	unsigned int swap;
	// NOP
	if (opcode == 0) {
		inst->op = NOP;
	}
	// MOVW
	else if (top8 == 0x1) {
		inst->op = MOVW;
		inst->D &= 0x0F;
		inst->K &= 0x0F;
	}
	// Signed and fractional multiply
	else if (top7 == 0x1) {
		// MULS
		if (top8 == 0x02) {
			inst->op = MULS;
			inst->D &= 0x0F;
			inst->K &= 0x0F;
		}
		else {
			swap = low8 & 0x88;
			inst->D &= 0x07;
			inst->K &= 0x07;
			// MULSU
			if (swap == 0x00) {
				inst->op = MULSU;
			}
			// FMUL
			else if (swap == 0x08) {
				inst->op = FMUL;
			}
			// FMULS
			else if (swap == 0x80) {
				inst->op = FMULS;
			}
			// FMULSU
			else if (swap == 0x88) {
				inst->op = FMULSU;
			}
			// NULL
			else {
				inst->op = NULL;
			}
		}
	}
	// 2-operand instructions
	// CPC
	else if (top6 == 0x01) {
		inst->op = CPC;	
	}
	// ADD
	else if (top6 == 0x03) {
		inst->op = ADD;
	}
	// SBC
	else if (top6 == 0x02) {
		inst->op = SBC;
	}
	// CPSE
	else if (top6 == 0x04) {
		inst->op = CPSE;
	}
	// CP
	else if (top6 == 0x05) {
		inst->op = CP;
	}
	// SUB
	else if (top6 == 0x06) {
		inst->op = SUB;
	}
	// ADC
	else if (top6 == 0x07) {
		inst->op = ADC;
	}
	// AND
	else if (top6 == 0x08) {
		inst->op = AND;
	}
	// EOR
	else if (top6 == 0x09) {
		inst->op = EOR;
	}
	// OR
	else if (top6 == 0x0A) {
		inst->op = OR;
	}
	// MOV
	else if (top6 == 0x0B) {
		inst->op = MOV;
	}
	// Register-immediate instructions
	// CPI
	else if (top4 == 0x3) {
		inst->op = CPI;
		inst->D &= 0x0F;
	}
	// SBCI
	else if (top4 == 0x4) {
		inst->op = SBCI;
		inst->D &= 0x0F;
	}
	// SUBI
	else if (top4 == 0x5) {
		inst->op = SUBI;
		inst->D &= 0x0F;
	}
	// ORI
	else if (top4 == 0x6) {
		inst->op = ORI;
		inst->D &= 0xF;
	}
	// ANDI
	else if (top4 == 0x7) {
		inst->op = ANDI;
		inst->D &= 0x0F;
	}
	// LDD and STD
	else if (top4 & 0xD == 0x8) {
		inst->K = (opcode & 0x7) | ((opcode >> 7) & 0x18)
			| ((opcode >> 8) & 0x20);
		inst->ireg = (low8 & 0x08) ? RY : RZ;
		swap = (opcode >> 9) & 0x01;
		// LDD
		if (swap == 0) {
			inst->op = LDD;
		}
		// STD
		else {
			inst-> = STD;
		}
	}
	// LDS and STS
	// Note that this is specificially the LDS
	// and STS not used by the ATtiny10 and similar
	// devices.
	else if (top6 == 0x24 && low4 == 0x0) {
		inst->op = (top7 & 0x01) ? ST : LD;
	}
	// PUSH and POP
	else if (top6 == 0x24 && low4 == 0xF) {
		inst->op = (top7 & 0x01) ? PUSH : POP;
	}
	// LD and ST
	// Also LAC/LAS/LAT/XCH for XMEGA
	// This Opcode must come after LDS/STS and PUSH/POP
	// To prevent matching their codes.
	else if (top7 == 0x24) {
		inst->mode = low8 & 0x03;
		inst->op = (top7 & 0x01) ? ST : LD;
		swap = (opcode >> 2) & 0x03;
		switch(swap) {
		case 0: // RZ 
			inst->ireg = RZ;
			if (opcode & 0x0F == 0) 
			break;
		case 1: // Weird XMEGA OPCODE and LPM and ELPM modes ii-iii
			// Override ST/LD
			if (inst->op == LD)
				switch(inst->mode) {
					case 0: // LPM (ii)
						inst->op = LPM;
						inst->mode = 2;
						break;
					case 1: // LPM (iii)
						inst->op = LPM;
						inst->mode = 3;
						break;
					case 2: // ELPM (ii)
						inst->op = ELPM;
						inst->mode = 2;
						break;
					case 3: // ELPM (iii)
						inst->op = ELPM;
						inst->mode = 3;
						break;
				}
			else
#ifdef XMEGA_SUPPORTED
				switch(inst->mode) {
				case 0: // XCH
					inst->op = XCH;
					break;
				case 1: // LAS
					inst->op = LAS;
					break;
				case 2: // LAC
					inst->op = LAC;
					break;
				case 3: // LAT
					inst->op = LAT;
					break;
				}
#else
				inst->op = NULL;
#endif
			break;
		case 2: // RY
			inst->ireg = RY;
			break;
		case 3: // RX
			inst->ireg = RX;
			break;
		} 
	}
	// 1-Operand Instructions
	else if (top7 == 0x4A && (opcode & 0x08) == 0) {
		swap = opcode & 0x0F;
		switch (swap) {
		case 0: // COM
			inst->op = COM;
			break;
		case 1: // NEG
			inst->op = NEG;
			break;
		case 2: // SWAP
			inst->op = SWAP;
			break;
		case 3: // INC
			inst->op = INC;
			break;
		case 5: // ASR
			inst->op = ASR;
			break;
		case 6: // LSR
			inst->op = LSR;
			break;
		case 7: // ROR
			inst->op = ROR;
			break;
		default: // NULL
			inst->op = NULL;

		}
	}
	// SEx and CLx
	else if (top8 == 0x94 && low4 == 0x8) {
		// The SEx and CLx commands are defined such
		// that they are an offset of the opcode
		// Use special SEx and CLx definitions
		inst->op = (inst->D & 0x80) ? CLx : SEx;
		inst->D = (~(inst->D >> 3)) & 0x01;
		inst->R = (opcode >> 4) & 0x07;
	}
	// Misc Instructions
	else if (top8 == 0x95 && low4 == 0x8) {
		swap = (opcode >> 4) & 0x0F;
		switch(swap) {
		case 0: // RET
			inst->op = RET;
			break;
		case 1: // RETI
			inst->op = RETI;
			break;
		case 8: // SLEEP
			inst->op = SLEEP;
			break;
		case 9: // BREAK
			inst->op = BREAK;
			break;
		case 0xA: // WDR
			inst->op = WDR;
			break;
		case 0xC: // LPM (i). ii-iii are located with LD and ST
			inst->LPM;
			inst->mode = 1;
			break;
		case 0xD: // ELPM (i). ii-iii are locaded with LD and ST
			inst->ELPM;
			inst->mode = 1;
			break;
		case 0xE: // SPM (i)
			inst->op = SPM;
			inst->mode = 1;
			break;
		case 0xF: // SPM (ii) aka SPM Z+
			inst->op = SPM;
			inst->mode = 2;
			break;
		default: // NULL
			inst->op = NULL;
		}
	}
	// JMP
	else if (top7 == 0x4A && low8 & 0x0E == 0x0C) {
		inst->op = JMP;
		inst->K = (opcode & 0x01) & ((opcode >> 3) & 0x3E);
	}
	// EIJMP
	else if (opcode == 0x9419) {
		inst->op = EIJMP;
	}
	// IJMP
	else if (opcdoe == 0x9409) {
		inst->op = IJMP;
	}
	// RJMP
	else if (top4 == 0xC) {
		inst->op = RJMP;
		inst->A = opcode & 0x0FFF;
	}
	// CALL
	else if (top7 == 0x4A && low8 & 0x0E == 0x0D) {
		inst->op = CALL;
		inst->K = (opcode & 0x01) & ((opcode >> 3) & 0x3E);
	}
	// EICALL
	else if (opcode == 0x9519) {
		inst->op = EICALL;
	}
	// ICALL
	else if (opcdoe == 0x9509) {
		inst->op = ICALL;
	}
	// RCALL
	else if (top4 == 0xD) {
		inst->op = RCALL;
		inst->A = opcode & 0x0FFF;
	}
	// DEC
	else if (opcode & 0xFE0F == 0x940A) {
		inst->op = DEC;
	}
	// DES
	else if (opcode & 0xFF0F == 0x940B) {
		inst->op = DES;
		inst->D &= 0x0F;
	}
	// ADIW
	else if (top8 == 0x96) {
		inst->op = ADIW;
		inst->K = (opcode & 0xF) & ((opcode >> 2) & 0x30);
		swap = (opcode >> 4) & 0x3;
		switch (swap) {
		case 0: // RW
			inst->ireg = RW;
			break;
		case 1: // RX
			inst->ireg = RX;
			break;
		case 2: // RY
			inst->ireg = RY;
			break;
		case 3: // RZ
			inst->ireg = RZ;
			break;
		}
	}
	// SBIW
	else if (top8 == 0x97) {
		inst->op = SBIW;
		inst->K = (opcode & 0xF) & ((opcode >> 2) & 0x30);
		swap = (opcode >> 4) & 0x3;
		switch (swap) {
		case 0: // RW
			inst->ireg = RW;
			break;
		case 1: // RX
			inst->ireg = RX;
			break;
		case 2: // RY
			inst->ireg = RY;
			break;
		case 3: // RZ
			inst->ireg = RZ;
			break;
		}
	}
	// I/O Bit Operations
	else if (top6 == 0x26) {
		// Uses R for bit index
		swap = top8 & 0x03;
		inst->A = (opcode >> 3) & 0x1F;
		inst->R &= 0x7;
		switch(swap) {
		case 0:	// CBI
			inst->op = CBI;
			break;
		case 1: // SBIC
			inst->op = SBIC;
			break;
		case 2:	// SBI
			inst->op SBI;
			break;
		case 3: //SBIS
			inst->op = SBIS;
			break;
		}
	}
	// Unsigned MUL
	else if (top6 == 0x27) {
		inst->op = MUL;
	}
	// IN/OUT
	else if (top4 == 0xB) {
		inst->op = (opcode & 0x0800) ? OUT : IN;
		inst->A = (opcode & 0x0F)  & ((opcode >> 5) & 0x03);
	}
	// LDI
	else if (top4 == 0xE) {
		// Uses D as h bits
		inst->op = LDI;
		inst->D &= 0x0F;
	}
	// SREG Conditional Branches
	else if (top5 == 0x1E) {
		inst->op = BRx;
		inst->R &= 0x07;
		inst->D &= (~(opcode >> 10)) & 0x01;
		inst->K = (opcode >> 2) & 0xFF;	// Line up sign bit with MSB
#if IS_ARITHMETIC_RS
		inst->K = inst-K >> 1;			// Perform arithmetic shift
#else
		swap = inst->K & 0x80;			// Extract sign bit
		inst->K = opcode >> 1;			// Perform bitwise shift
		inst->K |= swap;				// OR in sign bit
#endif
	} 
	// BLD and BST
	else if (top6 == 0x3E && (opcode & 0x08) == 0) {
		inst->op = (top7 & 0x01) ? BST : BLD;
		inst->R &= 0x07;
	}
	// SBRC and SBRS
	else if (top6 == 0x3F && (opcode & 0x08) == 0) {
		inst->op = (top7 & 0x01) ? SBRS : SBRC;
		inst->R &= 0x07;
	}
	else {
		inst->op = NULL;
	}
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
	//inst->D = (inst->D << 1) + 24;	// Calculate regs index
	//uint16_t *dword = (uint16*)(&(regs[inst->D]));
	inst->D += 12; // Calcuate index. d {24, 26, 28, 30}
	uint16_t dword = regps[inst->D];
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
	regps[inst->D] = res;
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
	inst->D |= 0x1F;	// Calculate index (16 <= d <= 31)
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

int SBC_run(Instruction *inst);
int SBCI_run(Instruction *inst);
int SBIW_run(Instruction *inst);

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
	inst->D |= 0x10;		// Calculate index
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

int OR_run(Instruction *inst);
int ORI_run(Instruction *inst);

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
	regs[inst->D] = ~regs[inst->D];
	// Calculate sreg
	sreg[VREG] = 0;
	CALC_N;
	CALC_Z;
	sreg[CREG] = 1;
	CALC_S;
	return 0;
}

int NEG_run(Instruction *inst);

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

int SER_run(Instruction *inst);

int MUL_run(Instruction *inst);
int MULS_run(Instruction *inst);
int MULSU_run(Instruction *inst);

// Fractional Multiply Unsigned
int FMUL_run(Instruction *inst) {
	inst->D += 16;	// Calculate the register
	inst->R += 16;	// indexes
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
	inst->D += 16;
	inst->R += 16;
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
	inst->D += 16;
	inst->R += 16;
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

int RJMP_run(Instruction *inst);
int IJMP_run(Instruction *inst);
int EIJMP_run(Instruction *inst);
int JMP_run(Instruction *inst);
int RCALL_run(Instruction *inst);
int EICALL_run(Instruction *inst);
int ICALL_run(Instruction *inst);
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
	regps[inst->D] = regps[inst->R];
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

