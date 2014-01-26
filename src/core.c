// core.c
//
// Developed by Charles Franklin
//
// MIT Lisense
//

#include <core.h>
#define IS_ARITHMETIC_RS (-2 >> 1 == -1)
#error MISC commands not implemented

void decodedInstruction(Instruction *inst, uint16_t opcode) {
	// Most variables are always in the same place if used
	// Get them now before conditional statements
	inst->D = (opcode >> 4) & 0x1F;
	inst->R = (opcode & 0x0F) | ((opcode >> 5) & 0x10);
	inst->K = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
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
	// LD and ST
	// Also LAC/LAS/LAT/XCH for XMEGA
	else if (top7 == 0x24 && low4 != 0x0 && low4 != 0xF) {
		inst->mode = low8 & 0x03;
		inst->op = (top7 & 0x01) ? ST : LD;
		swap = (opcode >> 2) & 0x03;
		switch(swap) {
		case 0: // RZ 
			inst->ireg = RZ;
			if (opcode & 0x0F == 0) 
			break;
		case 1: // Weird XMEGA OPCODE
#ifdef XMEGA_SUPPORTED
			// Override ST/LD
			if (inst->op == LD)
				inst->op = NULL;
			else
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
		inst->op = (isnt->D & 0x80) ? CLx : SEx;
		inst->D = (~(inst->D >> 3)) & 0x01;
		inst->R = (opcode >> 4) & 0x07;
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
