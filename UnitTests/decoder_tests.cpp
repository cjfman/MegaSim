// decoder_tests.cpp
//
//

#include <inttypes.h>
#include "gtest/gtest.h"
#include "opcode_defs.h"
#include "decoder.h"

uint16_t encode_rd(uint16_t rd) {
	return (rd & 0x1F) << 4;
}

uint16_t encode_4bit_rd(uint16_t rd) {
	return (rd & 0x0F) << 4;
}

uint16_t encode_rr(uint16_t rr) {
	return (rr & 0xF) | ((rr & 0x10) << 5);
}

uint16_t encode_4bit_rr(uint16_t rr) {
	return (rr & 0xF);
}

uint16_t encode_regs(uint16_t rr, uint16_t rd) {
	return encode_rr(rr) | encode_rd(rd);
}

uint16_t encode_4bit_regs(uint16_t rr, uint16_t rd) {
	return encode_4bit_rr(rr) | encode_4bit_rd(rd);
}

uint16_t encode_k(uint16_t k) {
	return (k & 0xF) | (k & 0xF0) << 4;
}

uint16_t encode_6bit_k(uint16_t k) {
	return (k & 0x0F) | (k & 0x30) << 2; 
}

#define EXPECT_ZERO(x) EXPECT_EQ(0, x)
#define EXPECT_INST(inst, op_L, R_L, D_L, mode_L, A_L, K_L, ireg_L, wsize_L) \
	EXPECT_EQ(op_L, inst.op); \
	EXPECT_EQ(R_L, inst.R); \
	EXPECT_EQ(D_L, inst.D); \
	EXPECT_EQ(mode_L, inst.mode); \
	EXPECT_EQ(A_L, inst.A); \
	EXPECT_EQ(ireg_L, inst.ireg); \
	EXPECT_EQ(wsize_L, inst.wsize);

#define EXPECT_INST_BASIC(inst, op_L, R_L, D_L) \
	EXPECT_EQ(op_L, inst.op); \
    EXPECT_EQ(R_L, inst.R); \
    EXPECT_EQ(D_L, inst.D); \
	EXPECT_EQ(1, inst.wsize);

#define EXPECT_INST_ONE(inst, op_L, D_L) \
	EXPECT_EQ(op_L, inst.op); \
    EXPECT_EQ(D_L, inst.D); \
	EXPECT_EQ(1, inst.wsize);

TEST(Instructions, newInstruction) {
	Instruction inst;
	makeBlankInstruction(&inst);
	EXPECT_ZERO(inst.op);
    EXPECT_ZERO(inst.R);
    EXPECT_ZERO(inst.D);
    EXPECT_ZERO(inst.mode);
    EXPECT_ZERO(inst.A);
    EXPECT_ZERO(inst.K);
    EXPECT_ZERO(inst.ireg);
    EXPECT_EQ(1, inst.wsize);
}

class DecoderF : public ::testing::Test {
public:
	Instruction inst;
	uint16_t rd, rr, code, k;
	virtual void SetUp() {
		makeBlankInstruction(&inst);
		rr = 0;
		rd = 0;
		code = 0;
	}
};

/////////////////////
// Arithmatic Codes
/////////////////////

// Opcode types
class DecoderOneOps : public DecoderF{};	// One operand
class DecoderTwoOps : public DecoderF{};	// Two operand
class DecoderImmOps : public DecoderF{};	// w/ immediates
class DecoderIndOps : public DecoderF{};	// w/ indirect registers
class DecoderMFSOps : public DecoderF{};	// Signed and fractional MUL
#ifdef XMEGA_SUPPORTED
class DecoderXmega :  public DecoderF{};	// XMEGA opcodes
#endif


uint16_t ADD_code = 0x0C00;

TEST_F(DecoderTwoOps, ADD_zero) {
	code = ADD_code;
	rr = 0;
	rd = 0;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, ADD, rr, rd);
}

TEST_F(DecoderTwoOps, ADD_num) {
	code = ADD_code;
	rr = 1;
	rd = 2;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, ADD, rr, rd);
}

uint16_t ADC_code = 0x1C00;

TEST_F(DecoderTwoOps, ADC_zero) {
	code = ADC_code;
	rr = 0;
	rd = 0;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, ADC, rr, rd);
}

TEST_F(DecoderTwoOps, ADC_num) {
	code = ADC_code;
	rr = 30;
	rd = 4;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, ADC, rr, rd);
}

uint16_t ADIW_code = 0x9600;

TEST_F(DecoderIndOps, ADIW_RW) {
    code = ADIW_code;
    rd = 0;	// RW
	k = 0x1A;
    code |= (rd & 0x3) << 4;
	code |= (k & 0x0F) | (k & 0x30) << 2; 
    decodeInstruction(&inst, &code);
	EXPECT_EQ(ADIW, inst.op);
	EXPECT_EQ(RW, inst.ireg);
	EXPECT_EQ(k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderIndOps, ADIW_RX) {
    code = ADIW_code;
    rd = 1;	// RX
	k = 0x08;
    code |= (rd & 0x3) << 4;
	code |= (k & 0x0F) | (k & 0x30) << 2; 
    decodeInstruction(&inst, &code);
	EXPECT_EQ(ADIW, inst.op);
	EXPECT_EQ(RX, inst.ireg);
	EXPECT_EQ(k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderIndOps, ADIW_RY) {
    code = ADIW_code;
    rd = 2;	// RY
	k = 0x35;
    code |= (rd & 0x3) << 4;
	code |= (k & 0x0F) | (k & 0x30) << 2; 
    decodeInstruction(&inst, &code);
	EXPECT_EQ(ADIW, inst.op);
	EXPECT_EQ(RY, inst.ireg);
	EXPECT_EQ(k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}


TEST_F(DecoderIndOps, ADIW_RZ) {
    code = ADIW_code;
    rd = 3;	// RZ
	k = 0x20;
    code |= (rd & 0x3) << 4;
	code |= (k & 0x0F) | (k & 0x30) << 2; 
    decodeInstruction(&inst, &code);
	EXPECT_EQ(ADIW, inst.op);
	EXPECT_EQ(RZ, inst.ireg);
	EXPECT_EQ(k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t SUB_code = 0x1800;

TEST_F(DecoderTwoOps, SUB_num) {
	code = SUB_code;
	rr = 3;
	rd = 29;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, SUB, rr, rd);
}

uint16_t SUBI_code = 0x5000;

TEST_F(DecoderImmOps, SUBI_zero) {
	code = SUBI_code;
	rd = 0;
	k = 127;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, &code);
	EXPECT_EQ(SUBI, inst.op);
	EXPECT_EQ(16, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderImmOps, SUBI_pos_k) {
	code = SUBI_code;
	rd = 20;
	k = 0x5A;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, &code);
	EXPECT_EQ(SUBI, inst.op);
	EXPECT_EQ(rd, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderImmOps, SUBI_maxneg_k) {
	code = SUBI_code;
	rd = 18;
	k = 128;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, &code);
	EXPECT_EQ(SUBI, inst.op);
	EXPECT_EQ(rd, inst.D);
	EXPECT_EQ(rd, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t SBC_code = 0x0800;

TEST_F(DecoderTwoOps, SBC_num) {
	code = SBC_code;
	rr = 4;
	rd = 5;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, SBC, rr, rd);
}

uint16_t SBCI_code = 0x4000;

TEST_F(DecoderImmOps, SBCI_neg1_k) {
	code = SBCI_code;
	rd = 17;
	k = 255;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, &code);
	EXPECT_EQ(SBCI, inst.op);
	EXPECT_EQ(rd, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t SBIW_code = 0x9700;

TEST_F(DecoderIndOps, SBIW_RW) {
    code = SBIW_code;
    rd = 0;	// RW
	k = 0x01;
    code |= (rd & 0x3) << 4;
	code |= encode_6bit_k(k);
    decodeInstruction(&inst, &code);
	EXPECT_EQ(SBIW, inst.op);
	EXPECT_EQ(RW, inst.ireg);
	EXPECT_EQ(k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderIndOps, SBIW_RX) {
    code = SBIW_code;
    rd = 1;	// RX
	k = 0x13;
    code |= (rd & 0x3) << 4;
	code |= encode_6bit_k(k);
    decodeInstruction(&inst, &code);
	EXPECT_EQ(SBIW, inst.op);
	EXPECT_EQ(RX, inst.ireg);
	EXPECT_EQ(k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderIndOps, SBIW_RY) {
    code = SBIW_code;
    rd = 2;	// RY
	k = 0x2C;
    code |= (rd & 0x3) << 4;
	code |= encode_6bit_k(k);
    decodeInstruction(&inst, &code);
	EXPECT_EQ(SBIW, inst.op);
	EXPECT_EQ(RY, inst.ireg);
	EXPECT_EQ(k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderIndOps, SBIW_RZ) {
    code = SBIW_code;
    rd = 3;	// RZ
	k = 0x2C;
    code |= (rd & 0x3) << 4;
	code |= encode_6bit_k(k);
    decodeInstruction(&inst, &code);
	EXPECT_EQ(SBIW, inst.op);
	EXPECT_EQ(RZ, inst.ireg);
	EXPECT_EQ(k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t AND_code = 0x2000;

TEST_F(DecoderTwoOps, AND_num) {
	code = AND_code;
	rd = 4;
	rr = 28;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, AND, rr, rd);
}

uint16_t ANDI_code = 0x7000;

TEST_F(DecoderImmOps, ANDI_zero_k) {
	code = ANDI_code;
	rd = 24;
	k = 0;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, &code);
	EXPECT_EQ(ANDI, inst.op);
	EXPECT_EQ(rd, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t ORI_code = 0x6000;

TEST_F(DecoderImmOps, ORI_wrap_rd) {
	code = ORI_code;
	rd = 6;
	k = 0x99;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, &code);
	rd |= 0x10;
	EXPECT_EQ(ORI, inst.op);
	EXPECT_EQ(rd, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t EOR_code = 0x2400;

TEST_F(DecoderTwoOps, EOR_num) {
	code = EOR_code;
	rd = 5;
	rr = 27;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, EOR, rr, rd);
}

uint16_t COM_code = 0x9400;

TEST_F(DecoderOneOps, COM_zero) {
	code = COM_code;
	rd = 0;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, COM, rd);
}

uint16_t NEG_code = 0x9401;

TEST_F(DecoderOneOps, NEG_max) {
	code = NEG_code;
	rd = 31;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, NEG, rd);
}

uint16_t INC_code = 0x9403;

TEST_F(DecoderOneOps, INC_num) {
	code = INC_code;
	rd = 3;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, INC, rd);
}

uint16_t DEC_code = 0x940A;

TEST_F(DecoderOneOps, DEC_num) {
	code = DEC_code;
	rd = 29;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, DEC, rd);
}

uint16_t MUL_code = 0x9C00;

TEST_F(DecoderTwoOps, MUL_num) {
	code = MUL_code;
	rd = 6;
	rr = 26;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, MUL, rr, rd);
}

uint16_t MULS_code = 0x0200;

TEST_F(DecoderMFSOps, MULS_zero) {
	code = MULS_code;
	rr = 0;
	rd = 0;
	code |= encode_4bit_regs(rr, rd);
	decodeInstruction(&inst, &code);
	rr |= 0x10;
	rd |= 0x10;
	EXPECT_INST_BASIC(inst, MULS, rr, rd);
}

TEST_F(DecoderMFSOps, MULS_wrap_rd) {
	code = MULS_code;
	rr = 17;
	rd = 4;
	code |= encode_4bit_regs(rr, rd);
	decodeInstruction(&inst, &code);
	rd |= 0x10;
	EXPECT_INST_BASIC(inst, MULS, rr, rd);
}

TEST_F(DecoderMFSOps, MULS_wrap_rr) {
	code = MULS_code;
	rr = 8;
	rd = 31;
	code |= encode_4bit_regs(rr, rd);
	decodeInstruction(&inst, &code);
	rr |= 0x10;
	EXPECT_INST_BASIC(inst, MULS, rr, rd);
}

uint16_t MULSU_code = 0x0300;

TEST_F(DecoderMFSOps, MULSU_zero) {
	code = MULSU_code;
	rr = 0;
	rd = 0;
	code |= encode_4bit_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, MULSU, 16, 16);
}

uint16_t FMUL_code = 0x0308;

TEST_F(DecoderMFSOps, FMUL_num) {
	code = FMUL_code;
	rr = 17;
	rd = 23;
	code |= encode_4bit_regs(rr, rd) & 0x77;
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, FMUL, rr, rd);
}

uint16_t FMULS_code = 0x0380;

TEST_F(DecoderMFSOps, FMULS_wrap_rd_up_rr_down) {
	code = FMULS_code;
	rr = 25;
	rd = 14;
	code |= encode_4bit_regs(rr, rd) & 0x77;
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, FMULS, 17, 22);
}

uint16_t FMULSU_code = 0x0388;

TEST_F(DecoderMFSOps, FMULSU_wrap_rr_up_rd_down) {
	code = FMULS_code;
	rr = 3;
	rd = 29;
	code |= encode_4bit_regs(rr, rd) & 0x77;
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, FMULS, 19, 21);
}

#ifdef XMEGA_SUPPORTED
uint16_t DES_code = 0x980B;

TEST_F(DecoderXmega, DES_zero) {
	code = DES_code;
	k = 0;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(DES, inst.op);
	EXPECT_EQ(k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderXmega, DES_num) {
	code = DES_code;
	k = 0xA;
	code |= (k & 0xF) << 4;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(DES, inst.op);
	EXPECT_EQ(k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}
#endif


/////////////////////////////
// Branch Instructions
/////////////////////////////

uint32_t encode_al(uint32_t al) {
	return (al & 0x10000) | ((al & 0x3E0000) << 3) | (al & 0xFFFF);
}

class DecoderBranchOps : public DecoderF {
public:
	int16_t a;
	virtual void SetUp () {
		DecoderF::SetUp();
		a = 0;
	};
};

#define EXPECT_R_INST(inst, op_L, a_L) \
	EXPECT_EQ(op_L, inst.op); \
	EXPECT_EQ(a_L, inst.A); \
	EXPECT_EQ(1, inst.wsize);

#define EXPECT_AL_INST(inst, op_L, al_L) \
	EXPECT_EQ(op_L, inst.op); \
	EXPECT_EQ(al_L, inst.AL); \
	EXPECT_EQ(2, inst.wsize);

uint16_t RJMP_code = 0xC000;

TEST_F(DecoderBranchOps, RJMP_zero) {
	code = RJMP_code;
	a = 0;
	decodeInstruction(&inst, &code);
	EXPECT_R_INST(inst, RJMP, a);
}

TEST_F(DecoderBranchOps, RJMP_pos) {
	code = RJMP_code;
	a = 0x04A5;
	code |= a & 0x0FFF;
	decodeInstruction(&inst, &code);
	EXPECT_R_INST(inst, RJMP, a);
}

TEST_F(DecoderBranchOps, RJMP_neg_one) {
	code = RJMP_code;
	a = -1;
	code |= a & 0x0FFF;
	decodeInstruction(&inst, &code);
	EXPECT_R_INST(inst, RJMP, a);
}

TEST_F(DecoderBranchOps, RJMP_neg) {
	code = RJMP_code;
	a = -168;
	code |= a & 0x0FFF;
	decodeInstruction(&inst, &code);
	EXPECT_R_INST(inst, RJMP, a);
}

uint16_t IJMP_code = 0x9409;

TEST_F(DecoderBranchOps, IJMP_call) {
	code = IJMP_code;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(IJMP, inst.op);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t EIJMP_code = 0x9419;

TEST_F(DecoderBranchOps, EIJMP_call) {
	code = EIJMP_code;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(EIJMP, inst.op);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t JMP_code = 0x940C;

TEST_F(DecoderBranchOps, JMP_zero) {
	uint16_t code[2];
	code[0] = JMP_code;
	code[1] = 0x0000;
	uint32_t al = 0x00000000;
	decodeInstruction(&inst, code);
	EXPECT_EQ(JMP, inst.op);
	EXPECT_EQ(al, inst.AL);
	EXPECT_EQ(2, inst.wsize);
}

TEST_F(DecoderBranchOps, JMP_num_1) {
	uint16_t code[2];
	code[0] = JMP_code;
	uint32_t al = 0x21A586;
	code[0] |= encode_al(al) >> 16;
	code[1] = encode_al(al) & 0xFFFF;
	decodeInstruction(&inst, code);
	EXPECT_EQ(JMP, inst.op);
	EXPECT_EQ(al, inst.AL);
	EXPECT_EQ(2, inst.wsize);
}

TEST_F(DecoderBranchOps, JMP_num_2) {
	uint16_t code[2];
	code[0] = JMP_code;
	uint32_t al = 0x1E586A;
	code[0] |= ((al & 0x10000) >> 16) | ((al & 0x3E0000) >> 13);
	code[1] = al & 0xFFFF;
	decodeInstruction(&inst, code);
	EXPECT_EQ(JMP, inst.op);
	EXPECT_EQ(al, inst.AL);
	EXPECT_EQ(2, inst.wsize);
}

uint16_t RCALL_code = 0xD000;

TEST_F(DecoderBranchOps, RCALL_zero) {
	code = RCALL_code;
	a = 0;
	decodeInstruction(&inst, &code);
	EXPECT_R_INST(inst, RCALL, a);
}

TEST_F(DecoderBranchOps, RCALL_pos) {
	code = RCALL_code;
	a = 0x035A;
	code |= a & 0x0FFF;
	decodeInstruction(&inst, &code);
	EXPECT_R_INST(inst, RCALL, a);
}

TEST_F(DecoderBranchOps, RCALL_neg_one) {
	code = RCALL_code;
	a = -1;
	code |= a & 0x0FFF;
	decodeInstruction(&inst, &code);
	EXPECT_R_INST(inst, RCALL, a);
}

TEST_F(DecoderBranchOps, RCALL_neg) {
	code = RCALL_code;
	a = -789;
	code |= a & 0x0FFF;
	decodeInstruction(&inst, &code);
	EXPECT_R_INST(inst, RCALL, a);
}
