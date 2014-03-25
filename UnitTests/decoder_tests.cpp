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

uint16_t encode_regs(uint16_t rr, uint16_t rd) {
	return encode_rr(rr) | encode_rd(rd);
}

uint16_t encode_k(uint16_t k) {
	return (k & 0xF) | (k & 0xF0) << 4;
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
    EXPECT_EQ(D_L, inst.D);

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
    EXPECT_ZERO(inst.wsize);
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

// Two operand codes
class DecoderTwoOps : public DecoderF{};
class DecoderImdOps : public DecoderF{};
class DecoderIndOps : public DecoderF{};


uint16_t ADD_code = 0x0C00;

TEST_F(DecoderTwoOps, ADD_zero) {
	code = ADD_code;
	rr = 0;
	rd = 0;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_BASIC(inst, ADD, rr, rd);
}

TEST_F(DecoderTwoOps, ADD_num) {
	code = ADD_code;
	rr = 1;
	rd = 2;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_BASIC(inst, ADD, rr, rd);
}

uint16_t ADC_code = 0x1C00;

TEST_F(DecoderTwoOps, ADC_zero) {
	code = ADC_code;
	rr = 0;
	rd = 0;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_BASIC(inst, ADC, rr, rd);
}

TEST_F(DecoderTwoOps, ADC_num) {
	code = ADC_code;
	rr = 30;
	rd = 4;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_BASIC(inst, ADC, rr, rd);
}

uint16_t ADIW_code = 0x9600;

TEST_F(DecoderIndOps, ADIW_RW) {
    code = ADIW_code;
    rd = 0;	// RW
	k = 0x1A;
    code |= (rd & 0x3) << 4;
	code |= (k & 0x0F) | (k & 0x30) << 2; 
    decodeInstruction(&inst, code);
	EXPECT_EQ(ADIW, inst.op);
	EXPECT_EQ(RW, inst.ireg);
}

TEST_F(DecoderIndOps, ADIW_RX) {
    code = ADIW_code;
    rd = 1;	// RX
	k = 0x08;
    code |= (rd & 0x3) << 4;
	code |= (k & 0x0F) | (k & 0x30) << 2; 
    decodeInstruction(&inst, code);
	EXPECT_EQ(ADIW, inst.op);
	EXPECT_EQ(RX, inst.ireg);
}

TEST_F(DecoderIndOps, ADIW_RY) {
    code = ADIW_code;
    rd = 2;	// RY
	k = 0x35;
    code |= (rd & 0x3) << 4;
	code |= (k & 0x0F) | (k & 0x30) << 2; 
    decodeInstruction(&inst, code);
	EXPECT_EQ(ADIW, inst.op);
	EXPECT_EQ(RY, inst.ireg);
}


TEST_F(DecoderIndOps, ADIW_RZ) {
    code = ADIW_code;
    rd = 3;	// RZ
	k = 0x20;
    code |= (rd & 0x3) << 4;
	code |= (k & 0x0F) | (k & 0x30) << 2; 
    decodeInstruction(&inst, code);
	EXPECT_EQ(ADIW, inst.op);
	EXPECT_EQ(RZ, inst.ireg);
}

uint16_t SUB_code = 0x1800;

TEST_F(DecoderTwoOps, SUB) {
	code = SUB_code;
	rr = 3;
	rd = 29;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_BASIC(inst, SUB, rr, rd);
}

uint16_t SUBI_code = 0x5000;

TEST_F(DecoderImdOps, SUBI_zero) {
	code = SUBI_code;
	rd = 0;
	k = 127;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, code);
	EXPECT_EQ(SUBI, inst.op);
	EXPECT_EQ((int8_t)k, inst.K);
}

TEST_F(DecoderImdOps, SUBI_maxneg) {
	code = SUBI_code;
	rd = 0;
	k = 128;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, code);
	EXPECT_EQ(SUBI, inst.op);
	EXPECT_EQ((int8_t)k, inst.K);
}

uint16_t SBC_code = 0x0800;

TEST_F(DecoderTwoOps, SBC) {
	code = SBC_code;
	rr = 4;
	rd = 5;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_BASIC(inst, SBC, rr, rd);
}
