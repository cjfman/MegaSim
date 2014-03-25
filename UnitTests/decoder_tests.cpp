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


uint16_t encode_rr(uint16_t rr) {
	return (rr & 0xF) | ((rr & 0x10) << 5);
}

uint16_t encode_regs(uint16_t rr, uint16_t rd) {
	return encode_rr(rr) | encode_rd(rd);
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
	uint16_t rd, rr, code;
	virtual void SetUp() {
		makeBlankInstruction(&inst);
		rr = 0;
		rd = 0;
		code = 0;
	}
};


uint16_t ADC_code = 0x1C00;

TEST_F(DecoderF, ADC_zero) {
	code = ADC_code;
	rr = 0;
	rd = 0;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_BASIC(inst, ADC, rr, rd);
}

TEST_F(DecoderF, ADC_num) {
	code = ADC_code;
	rr = 30;
	rd = 4;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_BASIC(inst, ADC, rr, rd);
}
