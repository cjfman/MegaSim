// decoder.c
//
// Developed by Charles Franklin
//
// MIT Lisense
//

#include "decoder.h"
#include "opcode_defs.h"

#ifdef DEBUG
#include <stdio.h>
void decoderIllop(const char* m) {
	printf("Decoder Error Illop: %s\n", m);
}
#endif

void makeBlankInstruction(Instruction *inst) {
	inst->op = 0;
	inst->R = 0;
	inst->D = 0;
	inst->mode = 0;
	inst->A = 0;
	inst->K = 0;
	inst->ireg = 0;
	inst->wsize = 0;
}

void decodeInstruction(Instruction *inst, uint16_t opcode) {
	// Most variables are always in the same place if used
	// Get them now before conditional statements
	inst->D = (opcode >> 4) & 0x1F;
	inst->R = (opcode & 0x0F) | ((opcode >> 5) & 0x10);
	inst->K = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
	// Other defaults
	inst->wsize = 1;
	// Get commonly used opcode bit vectors
	//unsigned int top9 = opcode >> 9;	
	unsigned int top8 = opcode >> 8;
	unsigned int top7 = opcode >> 9;
	unsigned int top6 = opcode >> 10;
	unsigned int top5 = opcode >> 11;
	unsigned int top4 = opcode >> 12;
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
			// ILLOP
			else {
#ifdef DEBUG
				decoderIllop("Signed and Fractional Multiply");
#endif
				inst->op = ILLOP;
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
		inst->D |= 0x10;	// 16 ≤ rd ≤ 31
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
	else if ((top4 & 0xD) == 0x8) {
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
			inst->op = STD;
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
			if ((opcode & 0x0F) == 0) 
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
				inst->op = ILLOP;
#ifdef DEBUG
				decoderIllop("XMEGA specific opcodes");
#endif // DEBUG
#endif // XMEGA_SUPPORTED
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
		default: // ILLOP
			inst->op = ILLOP;
#ifdef DEBUG
			decoderIllop("1 operand instructions");
#endif // DEBUG
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
			inst->op = LPM;
			inst->mode = 1;
			break;
		case 0xD: // ELPM (i). ii-iii are locaded with LD and ST
			inst->op = ELPM;
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
		default: // ILLOP
			inst->op = ILLOP;
#ifdef DEBUG
			decoderIllop("MISC instructions");
#endif // DEBUG
		}
	}
	// JMP
	else if ((top7 == 0x4A) && (low8 & 0x0E) == 0x0C) {
		inst->op = JMP;
		inst->A = (opcode & 0x01) & ((opcode >> 3) & 0x3E);
	}
	// EIJMP
	else if (opcode == 0x9419) {
		inst->op = EIJMP;
	}
	// IJMP
	else if (opcode == 0x9409) {
		inst->op = IJMP;
	}
	// RJMP
	else if (top4 == 0xC) {
		inst->op = RJMP;
		inst->A = opcode & 0x0FFF;
	}
	// CALL
	else if (top7 == 0x4A && (low8 & 0x0E) == 0x0D) {
		inst->op = CALL;
		inst->A = (opcode & 0x01) & ((opcode >> 3) & 0x3E);
	}
	// EICALL
	else if (opcode == 0x9519) {
		inst->op = EICALL;
	}
	// ICALL
	else if (opcode == 0x9509) {
		inst->op = ICALL;
	}
	// RCALL
	else if (top4 == 0xD) {
		inst->op = RCALL;
		inst->A = opcode & 0x0FFF;
	}
	// DEC
	else if ((opcode & 0xFE0F) == 0x940A) {
		inst->op = DEC;
	}
	// DES
	else if ((opcode & 0xFF0F) == 0x940B) {
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
			inst->op = SBI;
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
		inst->K = inst->K >> 1;			// Perform arithmetic shift
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
		inst->op = ILLOP;
#ifdef DEBUG
		decoderIllop("No match");
#endif // DEBUG
	}
}
