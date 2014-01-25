// core.c
//
// Developed by Charles Franklin
//
// MIT Lisense
//

#include <core.h>
#error MISC commands not implemented

void decodedInstruction(Instruction *inst, uint16_t opcode) {
	// Most variables are always in the same place if used
	// Get them now before conditional statements
	inst->D = (opcode >> 4) & 0x1F;
	inst->R = (opcode & 0x0F) | ((opcode >> 5) & 0x10);
	inst->K = (opcode & 0x0F) | ((opcode >> 4) & 0xF0);
	int top9 = opcode >> 9;	
	int top8 = opcode >> 8;
	int top7 = opcode >> 7;
	int top6 = opcode >> 6;
	int top5 = opcode >> 5;
	int top4 = opcode >> 4;
	int low8 = opcode & 0xFF;
	int low4 = opcode & 0x0F;
	int swap;
	// MOVW
	if (top8 == 0x1) {
		inst->op = MOVW;
	}
	// Signed and fractional multiply
	else if (top7 == 0x1) {
		// MULS
		if (top8 == 0x02) {
			inst->op = MULS;
		}
		else {
			swap = low8 & 0x88;
			// FMUL
			if (swap == 0x08) {
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
	else if (top4 == 0x03) {
		inst->op = CPI;
	}
	// SBCI
	else if (top4 == 0x04) {
		inst->op = SBCI;
	}
	// SUBI
	else if (top4 == 0x05) {
		inst->op = SUBI;
	}
	// ORI
	else if (top4 == 0x06) {
		inst->op = ORI;
	}
	// ANDI
	else if (top4 == 0x07) {
		inst->op = ANDI;
	}
	// LDD and STD
	else if (top4 & 0x0D == 0x08) {
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
	else if (top6 == 0x24) {
		inst->ireg = (low8 & 0x08) ? RY : RZ;
		inst->mode = low8 & 0x07;
		// LDD
		if (swap == 0) {
			inst->op = LD;
		}
		// STD
		else {
			inst-> = ST;
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
	else if (top8 == 0x94 && low4 == 0x08) {
		// The SEx and CLx commands are defined such
		// that they are an offset of the opcode
		// Use special SEx and CLx definitions
		inst->op = (isnt->D & 0x80) ? CLx : SEx;
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
	else if (top4 == 0x0C) {
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
	else if (top4 == 0x0D) {
		inst->op = RCALL;
		inst->A = opcode & 0x0FFF;
	}
}
