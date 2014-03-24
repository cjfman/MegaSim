// decoder_tests.cpp
//
//

#include <inttypes.h>
#include "gtest/gtest"
#include "decoder.h"

uint16_t encode_rd(uint16_t rd) {
	return (rd | 0x1F) << 4;
}


uint16_t encode_rr(uint16_t rr) {
	return (rd | 0xF) | ((rd | 0x10) << 5);
}

uint16_t encode_regs(uint16_t rr, uint16_t rd) {
	return encode_rr(rr) | encode_rd(rd);
}

#define EXPECT_ZERO(x) EXPECT_EQ(0, x)
#define EXPECT_INST(inst, op, R, D, mode, A, K, ireg, wsize) \
	EXPECT_EQ(op, inst.op); \
	EXPECT_EQ(R, inst.R); \
	EXPECT_EQ(D, inst.D); \
	EXPECT_EQ(mode, inst.mode); \
	EXPECT_EQ(A, inst.A); \
	EXPECT_EQ(ireg, inst.ireg); \
	EXPECT_EQ(wsize, isnt.wsize);

#define EXPECT_INST_BASIC(inst, op, R, D) \
	EXPECT_INST(inst, op, R, D, 0, 0, 0, 0, 0)

TEST(Instructions, newInstruction) {
	Intruction inst;
	makeBlankIntruction(&inst);
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
		makeBlankIntruction(&inst);
		rr = 0;
		rd = 0;
		code = 0;
	}
}


uint16_t ADC_code = 0x1C00;

TEST_F(Decoder, ADC_zero) {
	code = ADC_code;
	rr = 0;
	rd = 0;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, code);
	EXPECT_INST(inst, ADC, rr, rd);

TEST_F(Decoder, ADC_num) {
	code = ADC_code;
	rr = 30;
	rd = 4;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, code);
	EXPECT_INST(inst, ADC, rr, rd);
}
