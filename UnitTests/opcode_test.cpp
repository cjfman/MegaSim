// opcode_tests.cc
//
// Tests written for opcode_defs.c
//

#include "gtest/gtest.h"
#include "opcode_defs.h"

TEST(OpcodeTest, CoreLevels) {
	EXPECT_EQ(CORE_L_MIN, 76); 		// Minimum Core
	EXPECT_EQ(CORE_L_8k, 99); 		// Clasic Cores
	EXPECT_EQ(CORE_L_128k, 102);
	EXPECT_EQ(CORE_L_E8k,  111); 	// Enhanced Cores
	EXPECT_EQ(CORE_L_E128k, 112);
	EXPECT_EQ(CORE_L_E4M, 114);
	EXPECT_EQ(CORE_L_XMEGA, 119);  	// Mega core
	EXPECT_EQ(CORE_L_REDUC, 0);  	// Reduced // Unsupported for now
}

TEST(OpcodeTest, SupportedXMEGA) {
#ifdef XMEGA_SUPPORTED
	EXPECT_TRUE(false);
#else
	EXPECT_TRUE(true);
#endif
}

TEST(OpcodeTest, Opcodes) {
	EXPECT_EQ(ILLOP, 0);
	EXPECT_EQ(ADD,	33);
	EXPECT_EQ(ADC,	1);
	EXPECT_EQ(SUB,	2);
	EXPECT_EQ(SUBI,	3);
	EXPECT_EQ(SBC,	4);
	EXPECT_EQ(SBCI,	5);
	EXPECT_EQ(AND,	6);
	EXPECT_EQ(ANDI,	7);
	EXPECT_EQ(OR,	8);
	EXPECT_EQ(ORI,	9);
	EXPECT_EQ(EOR,	10);
	EXPECT_EQ(COM,	11);
	EXPECT_EQ(NEG,	12);
	//EXPECT_EQ(SBR,	13);
	//EXPECT_EQ(CBR,	14);
	EXPECT_EQ(INC,	15);
	EXPECT_EQ(DEC,	16);
	//EXPECT_EQ(TST,	17);	// Secretly AND
	//EXPECT_EQ(CLR,	18);	// Secretly EOR
	//EXPECT_EQ(SER,	19);
	EXPECT_EQ(RJMP,	20);
	EXPECT_EQ(RCALL,21);
	EXPECT_EQ(RET,	22);
	EXPECT_EQ(RETI,	23);
	EXPECT_EQ(CPSE,	24);
	EXPECT_EQ(CP,	25);
	EXPECT_EQ(CPC,	26);
	EXPECT_EQ(CPI,	27);
	EXPECT_EQ(SBRC,	28);
	EXPECT_EQ(SBRS,	29);
	EXPECT_EQ(SBIC,	30);
	EXPECT_EQ(SBIS,	31);
	EXPECT_EQ(BRx,	32);	// Covers all SREG conditional branches
	//EXPECT_EQ(BRBS,	32);	// Covered by the specific conditional branches
	//EXPECT_EQ(BRBC,	33);	// This one too. ADD borrows 33 for it's code.
	//EXPECT_EQ(BREQ,	34);
	//EXPECT_EQ(BRNE,	35);
	//EXPECT_EQ(BRCS,	36);
	//EXPECT_EQ(BRCC,	37);
	//EXPECT_EQ(BRSH,	38);	// aka BRCC
	//EXPECT_EQ(BRLO,	39);
	//EXPECT_EQ(BRMI,	40);
	//EXPECT_EQ(BRPL,	41);
	//EXPECT_EQ(BRGE,	42);
	//EXPECT_EQ(BRLT,	43);
	//EXPECT_EQ(BRHS,	44);
	//EXPECT_EQ(BRHC,	45);
	//EXPECT_EQ(BRTS,	46);
	//EXPECT_EQ(BRTC,	47);
	//EXPECT_EQ(BRVS,	48);
	//EXPECT_EQ(BRVC,	49);
	//EXPECT_EQ(BRIE,	50);
	//EXPECT_EQ(BRID,	51);
	EXPECT_EQ(LD,	52);
	EXPECT_EQ(ST,	53);
	EXPECT_EQ(MOV,	54);
	EXPECT_EQ(LDI,	55);
	EXPECT_EQ(IN,	56);
	EXPECT_EQ(OUT,	57);
	EXPECT_EQ(LPM,	58);
	EXPECT_EQ(SBI,	59);
	EXPECT_EQ(CBI,	60);
	//EXPECT_EQ(LSL,	61);	// Secretly ADD
	EXPECT_EQ(LSR,	62);
	//EXPECT_EQ(ROL,	63);	// Secretly ADC
	EXPECT_EQ(ROR,	64);
	EXPECT_EQ(ASR,	65);
	EXPECT_EQ(SWAP,	66);
	//EXPECT_EQ(BSET,	67);	// Covered by SEx
	//EXPECT_EQ(BCLR,	68);	// Covered by CLx
	EXPECT_EQ(BST,	69);
	EXPECT_EQ(BLD,	70);
	EXPECT_EQ(SEx,	71);	// The set and clear operations
	EXPECT_EQ(CLx,  72);	// are decoded differently. They
	//EXPECT_EQ(SEC,	71);	// Are decoded as SEx and CLx.
	//EXPECT_EQ(CLC,	72);
	//EXPECT_EQ(SEN,	73);	// The bits in the status register
	//EXPECT_EQ(CLN,	74);	// Are indexed in this order
	//EXPECT_EQ(SEZ,	75);	// +-------------------------------+
	//EXPECT_EQ(CLZ,	76);	// | I | T | H | S | V | N | Z | C |
	//EXPECT_EQ(SEI,	77);	// +-------------------------------+
	//EXPECT_EQ(CLI,	78);	//  MSB                         LSB
	//EXPECT_EQ(SES,	79);
	//EXPECT_EQ(CLS,	80);
	//EXPECT_EQ(SEV,	81);
	//EXPECT_EQ(CLV,	82);
	//EXPECT_EQ(SET,	83);
	//EXPECT_EQ(CLT,	84);
	//EXPECT_EQ(SEH,	85);
	//EXPECT_EQ(CLH,	86);
	EXPECT_EQ(NOP,	73);
	EXPECT_EQ(SLEEP,74);
	EXPECT_EQ(WDR,	75);
	EXPECT_EQ(ADIW,	90);
	EXPECT_EQ(SBIW,	91);
	EXPECT_EQ(IJMP,	92);
	EXPECT_EQ(ICALL,93);
	EXPECT_EQ(LDD,	94);
	EXPECT_EQ(LDS,	95);
	EXPECT_EQ(STD,  96);
	EXPECT_EQ(STS,	97);
	EXPECT_EQ(PUSH,	98);	// Secretly ST with mode 7
	EXPECT_EQ(POP,	99);	// Secretly LD with mode 7
	EXPECT_EQ(JMP,	100);
	EXPECT_EQ(CALL,	101);
	EXPECT_EQ(ELPM,	102);
	EXPECT_EQ(MUL,	103);
	EXPECT_EQ(MULS,	104);
	EXPECT_EQ(MULSU,105);
	EXPECT_EQ(FMUL,	106);
	EXPECT_EQ(FMULS,107);
	EXPECT_EQ(FMULSU,108);
	EXPECT_EQ(MOVW,	109);
	EXPECT_EQ(LPMen,110);
	EXPECT_EQ(SPM,	111);
	EXPECT_EQ(BREAK,112);
	EXPECT_EQ(EIJMP,113);
	EXPECT_EQ(EICALL,114);
	EXPECT_EQ(DES,	115);
	EXPECT_EQ(XCH,	116);
	EXPECT_EQ(LAS,	117);
	EXPECT_EQ(LAC,	118);
	EXPECT_EQ(LAT,	119);
}
