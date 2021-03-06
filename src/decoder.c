// decoder.c
//
// Developed by Charles Franklin
//
// MIT Lisense
//

#include <stdio.h>
#include <ctype.h>
#include "decoder.h"
#include "opcode_defs.h"
#include "core.h"
#include "devices.h"

#ifdef DEBUG
#define DEBUG_DECODER
#endif	// DEBUG

void decoderIllop(const char* m) {
	printf("Decoder Error Illop: %s\n", m);
}

void makeBlankInstruction(Instruction *inst) {
	inst->op = 0;
	inst->R = 0;
	inst->D = 0;
	inst->rrp = 0;
	inst->rdp = 0;
	inst->mode = 0;
	inst->A = 0;
	inst->K = 0;
	inst->ireg = 0;
	inst->wsize = 1;
	inst->cycles = 1;
}

void decodeAllInstructions(void) {
	unsigned int i;
	//uint16_t *data = program->data;
	//instruction *inst = program->instructions;
	for (i = 0; i < program->size; i++) {
		decodeInstruction(&program->instructions[i], &program->data[i]);
#ifdef DEBUG_DECODER
		fprintf(stderr, "0x%X\t0x%X\t", i, program->data[i]);
		printInstruction(&program->instructions[i]);
		fprintf(stderr, "\n");
#endif
	}
}

void printDisAsm(void) {
	int i;
	for (i = 0; i < program->size; i++) {
		fprintf(stderr, "0x%X\t0x%X\t", i, program->data[i]);
		printInstruction(&program->instructions[i]);
		fprintf(stderr, "\n");
	}
}

void printInstruction(Instruction *inst) {
	char *c;
	fprintf(stderr, "%s", opcode_strings[inst->op]);
	switch(inst->op) {
	// RD with word
	case LDS:
		fprintf(stderr, " R%d ", inst->D);
	case JMP:
	case CALL:
		fprintf(stderr, ": AL=0x%X", inst->AL);
		break;
	// RD with K
	case LDI:
		fprintf(stderr, " R%d ", inst->D);
	case RJMP:
	case RCALL:
	case BRx:
		fprintf(stderr, ": K=%d", inst->K);
		break;
	// RD to A
	case OUT:
		fprintf(stderr, " 0x%X, R%d", inst->A, inst->D);
		break;
	// A to RD
	case IN:
		fprintf(stderr, " R%d, 0x%X", inst->D, inst->A);
		break;
	// ireg and RD
	case STD:
		c = (inst->ireg == RW) ? "RW" :
				  (inst->ireg == RX) ? "RX" :
				  (inst->ireg == RY) ? "RY" :
				  (inst->ireg == RZ) ? "RZ" :
				                       "R?" ;
		fprintf(stderr, " %s+%d, R%d", c, inst->K, inst->D);
		break;
	// RD
	case POP:
	case PUSH:
		fprintf(stderr, " R%d", inst->D);
		break;
	// RD and RR
	case EOR:
	case MOV:
		fprintf(stderr, " R%d, R%d", inst->D, inst->R);
		break;
	case ILLOP:
		fprintf(stderr,": 0x%X '", *inst->ireg);
		unsigned char *c = (unsigned char*)inst->ireg;
		if (isprint(c[1]))
			fprintf(stderr, "%c", c[1]);
		else
			fprintf(stderr, "\\x%02x", c[1]);
		if (isprint(c[0]))
			fprintf(stderr, "%c'", c[0]);
		else
			fprintf(stderr, "\\x%02x'", c[0]);
		break;
	default:
		break;
	}
}

void decodeInstruction(Instruction *inst, uint16_t *opcode_p) {
	// Dereferece opcode
	uint16_t opcode = *opcode_p;
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
	unsigned int top4 = opcode >> 12;
	unsigned int swap;
	// NOP
	if (opcode == 0) {
		inst->op = NOP;
	}
	// MOVW
	else if ((opcode & 0xFF00) == 0x0100) {
		inst->op = MOVW;
		inst->D &= 0xF;
		inst->R &= 0xF;
		inst->D <<= 1; // Restrictred registers
		inst->R <<= 1; // Taken in pairs
	}
	// Signed and fractional multiply
	else if ((opcode & 0xFE00) == 0x0200) {
		inst->cycles = 2;
		// MULS
		if ((opcode & 0xFF00) == 0x0200) {
			inst->op = MULS;
			inst->D |= 0x10;	// Restricted register 16 ≤ rd ≤ 31
			inst->R |= 0x10;	// Restricted register 16 ≤ rr ≤ 31
		}
		else {
			inst->D &= 0x07;
			inst->D |= 0x10;	// Restricted register 16 ≤ rd ≤ 23
			inst->R &= 0x07;
			inst->R |= 0x10;	// Restricted register 16 ≤ rr ≤ 23
			// MULSU
			if ((opcode & 0xFF88) == 0x0300) {
				inst->op = MULSU;
			}
			// FMUL
			else if ((opcode & 0xFF88) == 0x0308) {
				inst->op = FMUL;
			}
			// FMULS
			else if ((opcode & 0xFF88) == 0x0380) {
				inst->op = FMULS;
			}
			// FMULSU
			else if ((opcode & 0xFF88) == 0x0388) {
				inst->op = FMULSU;
			}
			// ILLOP
			else {
#ifdef DEBUG_DECODER
				decoderIllop("Signed and Fractional Multiply");
#endif
				inst->op = ILLOP;
			}
		}
	}
	// 2-operand instructions
	// CPC
	else if ((opcode & 0xFC00) == 0x0400) {
		inst->op = CPC;	
	}
	// ADD
	else if ((opcode & 0xFC00) == 0x0C00) {
		inst->op = ADD;
	}
	// SBC
	else if ((opcode & 0xFC00) == 0x0800) {
		inst->op = SBC;
	}
	// CPSE
	else if ((opcode & 0xFC00) == 0x1000) {
		inst->op = CPSE;
	}
	// CP
	else if ((opcode & 0xFC00) == 0x1400) {
		inst->op = CP;
	}
	// SUB
	else if ((opcode & 0xFC00) == 0x1800) {
		inst->op = SUB;
	}
	// ADC
	else if ((opcode & 0xFC00) == 0x1C00) {
		inst->op = ADC;
	}
	// AND
	else if ((opcode & 0xFC00) == 0x2000) {
		inst->op = AND;
	}
	// EOR
	else if ((opcode & 0xFC00) == 0x2400) {
		inst->op = EOR;
	}
	// OR
	else if ((opcode & 0xFC00) == 0x2800) {
		inst->op = OR;
	}
	// MOV
	else if ((opcode & 0xFC00) == 0x2C00) {
		inst->op = MOV;
	}
	// Register-immediate instructions
	// CPI
	else if ((opcode & 0xF000) == 0x3000) {
		inst->op = CPI;
		inst->D |= 0x10;
	}
	// SBCI
	else if (top4 == 0x4) {
		inst->op = SBCI;
		inst->D |= 0x10;
	}
	// SUBI
	else if (top4 == 0x5) {
		inst->op = SUBI;
		inst->D |= 0x10;	// Restricted register 16 ≤ rd ≤ 31
	}
	// ORI
	else if (top4 == 0x6) {
		inst->op = ORI;
		inst->D |= 0x10;
	}
	// ANDI
	else if (top4 == 0x7) {
		inst->op = ANDI;
		inst->D |= 0x10;	// Restricted register 16 ≤ rd ≤ 31
	}
	// LDD and STD
	else if ((opcode & 0xD000) == 0x8000) {
		inst->K = (opcode & 0x7) | ((opcode & 0xC00) >> 7)
			| ((opcode & 0x2000) >> 8);
		inst->ireg = (opcode & 0x08) ? RY : RZ;
		swap = (opcode >> 9) & 0x01;
		// LDD and STD
		inst->op = (swap) ? STD : LDD;
		inst->cycles = 2;
	}
	// LDS and STS
	// Note that this is specificially the LDS
	// and STS not used by the ATtiny10 and similar
	// devices.
	else if ((opcode & 0xFC0F) == 0x9000) {
		inst->op = (opcode & 0x0200) ? STS : LDS;
		inst->AL = *(opcode_p + 1);
		inst->wsize = 2;
		inst->cycles = 2;
	}
	// PUSH and POP
	else if ((opcode & 0xFC0F) == 0x900F) {
		inst->op = (opcode & 0x0200) ? PUSH : POP;
		inst->cycles = 2;
	}
	// LD and ST
	// Also LAC/LAS/LAT/XCH for XMEGA
	// This Opcode must come after LDS/STS and PUSH/POP
	// To prevent matching their codes.
	else if ((opcode & 0xFC00) == 0x9000) {
		inst->cycles = 2;
		swap = ((opcode) & 0x03); // Get mode
		inst->mode = swap + 1;
		if (opcode & 0x0200) {
			inst->op = (swap == 0) ? ST  :
					   (swap == 1) ? ST2 :
					   (swap == 2) ? ST3 : ILLOP;
		}
		else {
			inst->op = (swap == 0) ? LD  :
					   (swap == 1) ? LD2 :
					   (swap == 2) ? LD3 : ILLOP;
		}
		swap = (opcode >> 2) & 0x03;
		switch(swap) {
		case 0: // RZ 
			inst->ireg = RZ;
			break;
		case 1: // Weird XMEGA OPCODE and LPM and ELPM modes ii-iii
			// Override ST/LD
			if (opcode & 0x0200) {
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
	#ifdef DEBUG_DECODER
				decoderIllop("XMEGA specific opcodes");
	#endif // DEBUG_DECODER
#endif // XMEGA_SUPPORTED
			}
			else {
				inst->cycles = 3;
				switch(opcode & 0x3) {
					case 0: // LPM (ii)
						inst->op = LPMen;
						inst->mode = 2;
						break;
					case 1: // LPM (iii)
						inst->op = LPMen;
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
			}
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
	else if ((opcode & 0xFE08) == 0x9400) {
		swap = opcode & 0x07;
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
#ifdef DEBUG_DECODER
			decoderIllop("1 operand instructions");
#endif // DEBUG_DECODER
		}
	}
	// SEx and CLx
	else if ((opcode & 0xFF0F) == 0x9408) {
		// The SEx and CLx commands are defined such
		// that they are an offset of the opcode
		// Use special SEx and CLx definitions
		inst->op = (opcode & 0x80) ? CLx : SEx;
		inst->R = (opcode >> 4) & 0x07;
	}
	// Misc Instructions
	else if ((opcode & 0xFF0F) == 0x9508) {
		swap = (opcode >> 4) & 0x0F;
		switch(swap) {
		case 0: // RET
			inst->op = RET;
			inst->cycles = (coredef->pc_size == 16) ? 4 : 5;
			break;
		case 1: // RETI
			inst->op = RETI;
			inst->cycles = (coredef->pc_size == 16) ? 4 : 5;
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
#ifdef DEBUG_DECODER
			decoderIllop("MISC instructions");
#endif // DEBUG_DECODER
		}
	}
	// JMP
	else if ((opcode & 0xFE0E) == 0x940C) {
		inst->op = JMP;
		inst->AL = (opcode & 0x01) | ((opcode & 0x01F0) >> 3);
		inst->AL = inst->AL << 16;
		inst->AL |= *(opcode_p + 1);	// Use next word
		inst->wsize = 2;
		inst->cycles = 3;
	}
	// EIJMP
	else if (opcode == 0x9419) {
		inst->op = EIJMP;
		inst->cycles = 2;
	}
	// IJMP
	else if (opcode == 0x9409) {
		inst->op = IJMP;
		inst->cycles = 2;
	}
	// RJMP
	else if ((opcode & 0xF000) == 0xC000) {
		inst->op = RJMP;
		inst->cycles = 2;
		inst->A = opcode & 0x0FFF;
#if IS_ARITHMETIC_RS
		inst->A = inst->A << 4;
		inst->A = inst->A >> 4;
#else
		swap = inst->A & 0x0800;
		if (swap) {
			inst->A |= 0xF000;
		}
#endif
	}
	// CALL
	else if ((opcode & 0xFE0E) == 0x940E) {
		inst->op = CALL;
		inst->cycles = (coredef->pc_size == 16) ? 4 : 5;
		inst->AL = (opcode & 0x01) | ((opcode & 0x01F0) >> 3);
		inst->AL = inst->AL << 16;
		inst->AL |= *(opcode_p + 1);	// Use next word
		inst->wsize = 2;
	}
	// EICALL
	else if (opcode == 0x9519) {
		inst->op = EICALL;
		inst->cycles = 4;
	}
	// ICALL
	else if (opcode == 0x9509) {
		inst->op = ICALL;
		inst->cycles = (coredef->pc_size == 16) ? 3 : 4;
	}
	// RCALL
	else if ((opcode & 0xF000) == 0xD000) {
		inst->op = RCALL;
		inst->cycles = (coredef->pc_size == 16) ? 3 : 4;
		inst->A = opcode & 0x0FFF;
#if IS_ARITHMETIC_RS
		inst->A = inst->A << 4;
		inst->A = inst->A >> 4;
#else
		swap = inst->A & 0x0800;
		if (swap) {
			inst->A |= 0xF000;
		}
#endif
	}
	// DEC
	else if ((opcode & 0xFE0F) == 0x940A) {
		inst->op = DEC;
	}
#ifdef XMEGA_SUPPORTED
	// DES
	else if ((opcode & 0xFF0F) == 0x940B) {
		inst->op = DES;
		inst->D &= 0x0F;
	}
#endif
	// ADIW
	else if (top8 == 0x96) {
		inst->op = ADIW;
		inst->K = (opcode & 0xF) | ((opcode >> 2) & 0x30);
		inst->cycles = 2;
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
		inst->K = (opcode & 0xF) | ((opcode >> 2) & 0x30);
		inst->cycles = 2;
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
	else if ((opcode & 0xFC00) == 0x9800) {
		// Uses R for bit index
		swap = (opcode >> 8) & 0x03;
		inst->A = (opcode >> 3) & 0x1F;
		inst->R &= 0x7;
		switch(swap) {
		case 0:	// CBI
			inst->op = CBI;
			inst->cycles = 2;
			break;
		case 1: // SBIC
			inst->op = SBIC;
			break;
		case 2:	// SBI
			inst->op = SBI;
			inst->cycles = 2;
			break;
		case 3: //SBIS
			inst->op = SBIS;
			break;
		}
	}
	// Unsigned MUL
	else if ((opcode & 0xFC00) == 0x9C00) {
		inst->op = MUL;
		inst->cycles = 2;
	}
	// IN/OUT
	else if ((opcode & 0xF000) == 0xB000) {
		inst->op = (opcode & 0x0800) ? OUT : IN;
		inst->A = (opcode & 0x0F) | ((opcode & 0x0600) >> 5);
	}
	// LDI
	else if (top4 == 0xE) {
		// Uses D as h bits
		inst->op = LDI;
		inst->D |= 0x10;
	}
	// SREG Conditional Branches
	else if ((opcode & 0xF800) == 0xF000) {
		inst->op = BRx;
		inst->R &= 0x07;
		inst->D = (~(opcode >> 10)) & 0x01;	// Set if branch when set
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
	else if ((opcode & 0xFC00) == 0xFC00) {
		inst->op = (opcode & 0x0200) ? SBRS : SBRC;
		inst->R &= 0x07;
	}
	else {
		inst->op = ILLOP;
#ifdef DEBUG_DECODER
		decoderIllop("No match");
#endif // DEBUG_DECODER
	}

	// Unsupported Code Detection
	if (coredef->type < inst->op) {
		inst->op = ILLOP;
	}

	if (inst->op == ILLOP) {
		// Point ireg to the illegal op
		inst->ireg = opcode_p;
	}
	inst->rrp = regs + inst->R;
	inst->rdp = regs + inst->D;
}
