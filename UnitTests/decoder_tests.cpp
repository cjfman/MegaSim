// decoder_tests.cpp
//
//

#include <inttypes.h>
#include "gtest/gtest.h"
#include "opcode_defs.h"
#include "decoder.h"
#include "core.h"

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

#define EXPECT_INST_BASIC(inst, op_L) \
	EXPECT_EQ(op_L, inst.op); \
	EXPECT_EQ(1, inst.wsize);

#define EXPECT_INST_TWO(inst, op_L, R_L, D_L) \
	EXPECT_EQ(op_L, inst.op); \
    EXPECT_EQ(R_L, inst.R); \
    EXPECT_EQ(D_L, inst.D); \
	EXPECT_EQ(1, inst.wsize);

#define EXPECT_INST_ONE(inst, op_L, D_L) \
	EXPECT_EQ(op_L, inst.op); \
    EXPECT_EQ(D_L, inst.D); \
	EXPECT_EQ(1, inst.wsize);

#define EXPECT_INST_IMM(inst, op_L, D_L, K_L) \
	EXPECT_EQ(op_L, inst.op); \
	EXPECT_EQ(D_L, inst.D); \
	EXPECT_EQ((int8_t)K_L, inst.K); \
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
	EXPECT_INST_TWO(inst, ADD, rr, rd);
}

TEST_F(DecoderTwoOps, ADD_num) {
	code = ADD_code;
	rr = 1;
	rd = 2;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, ADD, rr, rd);
}

uint16_t ADC_code = 0x1C00;

TEST_F(DecoderTwoOps, ADC_zero) {
	code = ADC_code;
	rr = 0;
	rd = 0;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, ADC, rr, rd);
}

TEST_F(DecoderTwoOps, ADC_num) {
	code = ADC_code;
	rr = 30;
	rd = 4;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, ADC, rr, rd);
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
	EXPECT_INST_TWO(inst, SUB, rr, rd);
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
	EXPECT_INST_TWO(inst, SBC, rr, rd);
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
	EXPECT_INST_TWO(inst, AND, rr, rd);
}

uint16_t ANDI_code = 0x7000;

TEST_F(DecoderImmOps, ANDI_zero_k) {
	code = ANDI_code;
	rd = 24;
	k = 0;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, &code);
	EXPECT_INST_IMM(inst, ANDI, rd, k);
}

uint16_t ORI_code = 0x6000;

TEST_F(DecoderImmOps, ORI_wrap_rd) {
	code = ORI_code;
	rd = 6;
	k = 0x99;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, &code);
	rd |= 0x10;
	EXPECT_INST_IMM(inst, ORI, rd, k);
}

uint16_t EOR_code = 0x2400;

TEST_F(DecoderTwoOps, EOR_num) {
	code = EOR_code;
	rd = 5;
	rr = 27;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, EOR, rr, rd);
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
	EXPECT_INST_TWO(inst, MUL, rr, rd);
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
	EXPECT_INST_TWO(inst, MULS, rr, rd);
}

TEST_F(DecoderMFSOps, MULS_wrap_rd) {
	code = MULS_code;
	rr = 17;
	rd = 4;
	code |= encode_4bit_regs(rr, rd);
	decodeInstruction(&inst, &code);
	rd |= 0x10;
	EXPECT_INST_TWO(inst, MULS, rr, rd);
}

TEST_F(DecoderMFSOps, MULS_wrap_rr) {
	code = MULS_code;
	rr = 8;
	rd = 31;
	code |= encode_4bit_regs(rr, rd);
	decodeInstruction(&inst, &code);
	rr |= 0x10;
	EXPECT_INST_TWO(inst, MULS, rr, rd);
}

uint16_t MULSU_code = 0x0300;

TEST_F(DecoderMFSOps, MULSU_zero) {
	code = MULSU_code;
	rr = 0;
	rd = 0;
	code |= encode_4bit_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, MULSU, 16, 16);
}

uint16_t FMUL_code = 0x0308;

TEST_F(DecoderMFSOps, FMUL_num) {
	code = FMUL_code;
	rr = 17;
	rd = 23;
	code |= encode_4bit_regs(rr, rd) & 0x77;
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, FMUL, rr, rd);
}

uint16_t FMULS_code = 0x0380;

TEST_F(DecoderMFSOps, FMULS_wrap_rd_up_rr_down) {
	code = FMULS_code;
	rr = 25;
	rd = 14;
	code |= encode_4bit_regs(rr, rd) & 0x77;
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, FMULS, 17, 22);
}

uint16_t FMULSU_code = 0x0388;

TEST_F(DecoderMFSOps, FMULSU_wrap_rr_up_rd_down) {
	code = FMULS_code;
	rr = 3;
	rd = 29;
	code |= encode_4bit_regs(rr, rd) & 0x77;
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, FMULS, 19, 21);
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
	EXPECT_AL_INST(inst, JMP, al);
}

TEST_F(DecoderBranchOps, JMP_num_1) {
	uint16_t code[2];
	code[0] = JMP_code;
	uint32_t al = 0x21A586;
	code[0] |= encode_al(al) >> 16;
	code[1] = encode_al(al) & 0xFFFF;
	decodeInstruction(&inst, code);
	EXPECT_AL_INST(inst, JMP, al);
}

TEST_F(DecoderBranchOps, JMP_num_2) {
	uint16_t code[2];
	code[0] = JMP_code;
	uint32_t al = 0x1E586A;
	code[0] |= encode_al(al) >> 16;
	code[1] = encode_al(al) & 0xFFFF;
	decodeInstruction(&inst, code);
	EXPECT_AL_INST(inst, JMP, al);
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

uint16_t ICALL_code = 0x9509;

TEST_F(DecoderBranchOps, ICALL_call) {
	code = ICALL_code;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(ICALL, inst.op);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t EICALL_code = 0x9519;

TEST_F(DecoderBranchOps, EICALL_call) {
	code = EICALL_code;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(EICALL, inst.op);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t CALL_code = 0x940E;

TEST_F(DecoderBranchOps, CALL_zero) {
	uint16_t code[2];
	code[0] = CALL_code;
	code[1] = 0x0000;
	uint32_t al = 0x00000000;
	decodeInstruction(&inst, code);
	EXPECT_AL_INST(inst, CALL, al);
}

TEST_F(DecoderBranchOps, CALL_num_1) {
	uint16_t code[2];
	code[0] = CALL_code;
	uint32_t al = 0x21A586;
	code[0] |= encode_al(al) >> 16;
	code[1] = encode_al(al) & 0xFFFF;
	decodeInstruction(&inst, code);
	EXPECT_AL_INST(inst, CALL, al);
}

TEST_F(DecoderBranchOps, CALL_num_2) {
	uint16_t code[2];
	code[0] = CALL_code;
	uint32_t al = 0x1E586A;
	code[0] |= encode_al(al) >> 16;
	code[1] = encode_al(al) & 0xFFFF;
	decodeInstruction(&inst, code);
	EXPECT_AL_INST(inst, CALL, al);
}

uint16_t RET_code = 0x9508;

TEST_F(DecoderBranchOps, RET_call) {
	code = RET_code;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(RET, inst.op);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t RETI_code = 0x9518;

TEST_F(DecoderBranchOps, RETI_call) {
	code = RETI_code;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(RETI, inst.op);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t CPSE_code = 0x1000;

TEST_F(DecoderTwoOps, CPSE_num) {
	code = CPSE_code;
	rd = 7;
	rr = 27;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, CPSE, rr, rd);
}

uint16_t CP_code = 0x1400;

TEST_F(DecoderTwoOps, CP_num) {
	code = CP_code;
	rd = 8;
	rr = 26;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, CP, rr, rd);
}

uint16_t CPC_code = 0x0400;

TEST_F(DecoderTwoOps, CPC_num) {
	code = CPC_code;
	rd = 9;
	rr = 25;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, CPC, rr, rd);
}

uint16_t CPI_code = 0x3000;

TEST_F(DecoderImmOps, CPI_zero) {
	code = CPI_code;
	rd = 0;
	k = 0;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, &code);
	rd |= 0x10;
	EXPECT_EQ(CPI, inst.op);
	EXPECT_EQ(rd, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderImmOps, CPI_num) {
	code = CPI_code;
	rd = 18;
	k = 0x7E;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, &code);
	rd |= 0x10;
	EXPECT_EQ(CPI, inst.op);
	EXPECT_EQ(rd, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderImmOps, CPI_neg_k) {
	code = CPI_code;
	rd = 30;
	k = -1;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, &code);
	rd |= 0x10;
	EXPECT_EQ(CPI, inst.op);
	EXPECT_EQ(rd, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderImmOps, CPI_wrap_rd) {
	code = CPI_code;
	rd = 9;
	k = -6;
	code |= encode_4bit_rd(rd) | encode_k(k);
	decodeInstruction(&inst, &code);
	rd |= 0x10;
	EXPECT_EQ(CPI, inst.op);
	EXPECT_EQ(rd, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t SBRC_code = 0xFC00;

TEST_F(DecoderBranchOps, SBRC_zero) {
	code = SBRC_code;
	rd = 0;
	rr = 0;
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, SBRC, rr, rd);
}

TEST_F(DecoderBranchOps, SBRC_num) {
	code = SBRC_code;
	rd = 0x15;
	rr = 4;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, SBRC, rr, rd);
}

TEST_F(DecoderBranchOps, SBRC_wrap_rr) {
	code = SBRC_code;
	rd = 0x0A;
	rr = 24;
	code |= encode_regs(rr & 0x7, rd);
	decodeInstruction(&inst, &code);
	rr &= 0x7;
	EXPECT_INST_TWO(inst, SBRC, rr, rd);
}

uint16_t SBRS_code = 0xFE00;

TEST_F(DecoderBranchOps, SBRS_zero) {
	code = SBRS_code;
	rd = 0;
	rr = 0;
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, SBRS, rr, rd);
}

TEST_F(DecoderBranchOps, SBRS_num) {
	code = SBRS_code;
	rd = 0x19;
	rr = 7;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, SBRS, rr, rd);
}

TEST_F(DecoderBranchOps, SBRS_wrap_rr) {
	code = SBRS_code;
	rd = 0x06;
	rr = 31;
	code |= encode_regs(rr & 0x7, rd);
	decodeInstruction(&inst, &code);
	rr &= 0x7;
	EXPECT_INST_TWO(inst, SBRS, rr, rd);
}

uint16_t SBIC_code = 0x9900;

TEST_F(DecoderBranchOps, SBIC_zero) {
	code = SBIC_code;
	rr = 0;
	a = 0;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(SBIC, inst.op);
	EXPECT_EQ(a, inst.A);
	EXPECT_EQ(rr, inst.R);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderBranchOps, SBIC_num) {
	code = SBIC_code;
	rr = 5;
	a = 0x1A;
	code |= (rr & 0x7) | (a << 3);
	decodeInstruction(&inst, &code);
	EXPECT_EQ(SBIC, inst.op);
	EXPECT_EQ(a, inst.A);
	EXPECT_EQ(rr, inst.R);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderBranchOps, SBIC_wrap) {
	code = SBIC_code;
	rr = 5;
	a = 0x05;
	code |= (rr & 0x7) | (a << 3);
	rr &= 0x7;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(SBIC, inst.op);
	EXPECT_EQ(a, inst.A);
	EXPECT_EQ(rr, inst.R);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t SBIS_code = 0x9B00;

TEST_F(DecoderBranchOps, SBIS_zero) {
	code = SBIS_code;
	rr = 0;
	a = 0;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(SBIS, inst.op);
	EXPECT_EQ(a, inst.A);
	EXPECT_EQ(rr, inst.R);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderBranchOps, SBIS_num) {
	code = SBIS_code;
	rr = 1;
	a = 0x19;
	code |= (rr & 0x7) | (a << 3);
	decodeInstruction(&inst, &code);
	EXPECT_EQ(SBIS, inst.op);
	EXPECT_EQ(a, inst.A);
	EXPECT_EQ(rr, inst.R);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderBranchOps, SBIS_wrap) {
	code = SBIS_code;
	rr = 3;
	a = 0x06;
	code |= (rr & 0x7) | (a << 3);
	rr &= 0x7;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(SBIS, inst.op);
	EXPECT_EQ(a, inst.A);
	EXPECT_EQ(rr, inst.R);
	EXPECT_EQ(1, inst.wsize);
}

uint16_t BRx_code = 0xF000;

TEST_F(DecoderBranchOps, BRBC_zero) {
	code = BRx_code | 0x0400;
	k = 0;
	rr = 0;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(BRx, inst.op);
	EXPECT_EQ(rr, inst.R);
	EXPECT_EQ(0, inst.D);
	EXPECT_EQ(k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderBranchOps, BRBC_neg) {
	code = BRx_code | 0x0400;
	k = 0xDA;
	rr = 5;
	code |= rr | (k & 0x7F) << 3;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(BRx, inst.op);
	EXPECT_EQ(rr, inst.R);
	EXPECT_EQ(0, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderBranchOps, BRBC_num) {
	code = BRx_code | 0x0400;
	k = 0x15;
	rr = 2;
	code |= rr | (k & 0x7F) << 3;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(BRx, inst.op);
	EXPECT_EQ(rr, inst.R);
	EXPECT_EQ(0, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderBranchOps, BRBS_zero) {
	code = BRx_code;
	k = 0;
	rr = 0;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(BRx, inst.op);
	EXPECT_EQ(rr, inst.R);
	EXPECT_EQ(1, inst.D);
	EXPECT_EQ(k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderBranchOps, BRBS_neg) {
	code = BRx_code;
	k = 0xDA;
	rr = 5;
	code |= rr | (k & 0x7F) << 3;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(BRx, inst.op);
	EXPECT_EQ(rr, inst.R);
	EXPECT_EQ(1, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}

TEST_F(DecoderBranchOps, BRBS_num) {
	code = BRx_code;
	k = 0x15;
	rr = 2;
	code |= rr | (k & 0x7F) << 3;
	decodeInstruction(&inst, &code);
	EXPECT_EQ(BRx, inst.op);
	EXPECT_EQ(rr, inst.R);
	EXPECT_EQ(1, inst.D);
	EXPECT_EQ((int8_t)k, inst.K);
	EXPECT_EQ(1, inst.wsize);
}


//////////////////////////////
// Data Transfer Instructions
//////////////////////////////

#define EXPECT_INST_ONE_AL(inst, op_L, d, al) \
	EXPECT_EQ(op_L, inst.op); \
	EXPECT_EQ(d, inst.D); \
	EXPECT_EQ(al, inst.AL); \
	EXPECT_EQ(2, inst.wsize);

#define EXPECT_INST_ONE_A(inst, op_L, d, a) \
	EXPECT_EQ(op_L, inst.op); \
	EXPECT_EQ(d, inst.D); \
	EXPECT_EQ(a, inst.A); \
	EXPECT_EQ(1, inst.wsize);

#define EXPECT_INST_DATA(inst, op_L, d, reg, mode_L) \
	EXPECT_EQ(op_L, inst.op); \
	EXPECT_EQ(d, inst.D); \
	EXPECT_EQ(reg, inst.ireg); \
	EXPECT_EQ(mode_L, inst.mode); \
	EXPECT_EQ(1, inst.wsize);

#define EXPECT_INST_Q(inst, op_L, d, q) \
	EXPECT_EQ(op_L, inst.op); \
	EXPECT_EQ(d, inst.D); \
	EXPECT_EQ(q, inst.K); \
	EXPECT_EQ(1, inst.wsize);

#define EXPECT_INST_ONE_MODE(inst, op_L, d, mode_L) \
	EXPECT_EQ(op_L, inst.op); \
	EXPECT_EQ(d, inst.D); \
	EXPECT_EQ(mode_L, inst.mode); \
	EXPECT_EQ(1, inst.wsize);

uint16_t encode_mode(uint8_t mode) {
	return (mode - 1) & 0x3;
}

uint16_t encode_q(uint16_t q) {
	return (q & 0x7) | ((q & 0x18) << 7) | ((q & 0x20) << 8);
}

class DecoderDataOps : public DecoderF {
public:
	uint8_t mode;
	uint8_t q;
	uint8_t a;
	uint16_t al;
	virtual void SetUp() {
		DecoderF::SetUp();
		al = 0;
		mode = 0;
		q = 0;
	};
};

uint16_t MOV_code = 0x2C00;

TEST_F(DecoderTwoOps, MOV_num) {
	code = MOV_code;
	rd = 10;
	rr = 24;
	code |= encode_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, MOV, rr, rd);
}

uint16_t MOVW_code = 0x0100;

TEST_F(DecoderTwoOps, MOVW_zero) {
	code = MOVW_code;
	rd = 0;
	rr = 0;
	code |= encode_4bit_regs(rr, rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_TWO(inst, MOVW, rr, rd);
}

TEST_F(DecoderTwoOps, MOVW_num_1) {
	code = MOVW_code;
	rd = 0xA;
	rr = 0x5;
	code |= encode_4bit_regs(rr, rd);
	decodeInstruction(&inst, &code);
	rd <<= 1;
	rr <<= 1;
	EXPECT_INST_TWO(inst, MOVW, rr, rd);
}

TEST_F(DecoderTwoOps, MOVW_num_2) {
	code = MOVW_code;
	rd = 0x5;
	rr = 0xA;
	code |= encode_4bit_regs(rr, rd);
	decodeInstruction(&inst, &code);
	rd <<= 1;
	rr <<= 1;
	EXPECT_INST_TWO(inst, MOVW, rr, rd);
}

uint16_t LDI_code = 0xE000;

TEST_F(DecoderImmOps, LDI_zero) {
	code = LDI_code;
	rd = 0;
	k = 0;
	decodeInstruction(&inst, &code);
	rd |= 0x10;
	EXPECT_INST_IMM(inst, LDI, rd, k);
}

TEST_F(DecoderImmOps, LDI_neg) {
	code = LDI_code;
	rd = 0xA;
	k = 0xAA;
	code |= encode_k(k) | encode_4bit_rd(rd);
	decodeInstruction(&inst, &code);
	rd |= 0x10;
	EXPECT_INST_IMM(inst, LDI, rd, k);
}

TEST_F(DecoderImmOps, LDI_pos) {
	code = LDI_code;
	rd = 0x5;
	k = 0x55;
	code |= encode_k(k) | encode_4bit_rd(rd);
	decodeInstruction(&inst, &code);
	rd |= 0x10;
	EXPECT_INST_IMM(inst, LDI, rd, k);
}

uint16_t LDS_code = 0x9000;

TEST_F(DecoderDataOps, LDS_zero) {
	uint16_t code[2];
	code[0] = LDS_code;
	rd = 0;
	al = 0x0000;
	code[1] = al;
	code[0] |= encode_rd(rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_ONE_AL(inst, LDS, rd, al);
}

TEST_F(DecoderDataOps, LDS_num_1) {
	uint16_t code[2];
	code[0] = LDS_code;
	rd = 0xA;
	al = 0xA596;
	code[1] = al;
	code[0] |= encode_rd(rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_ONE_AL(inst, LDS, rd, al);
}

TEST_F(DecoderDataOps, LDS_num_2) {
	uint16_t code[2];
	code[0] = LDS_code;
	rd = 0x15;
	al = 0x5A69;
	code[1] = al;
	code[0] |= encode_rd(rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_ONE_AL(inst, LDS, rd, al);
}

uint16_t LD_code = 0x9000;

TEST_F(DecoderDataOps, LD_RX_1) {
	code = LD_code | 0xC; //RX
	rd = 0;
	mode = 1;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, LD, rd, RX, mode);
}

TEST_F(DecoderDataOps, LD_RX_2) {
	code = LD_code | 0xC; //RX
	rd = 0x06;
	mode = 2;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, LD, rd, RX, mode);
}

TEST_F(DecoderDataOps, LD_RX_3) {
	code = LD_code | 0xC; //RX
	rd = 0x19;
	mode = 3;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, LD, rd, RX, mode);
}

TEST_F(DecoderDataOps, LD_RY_2) {
	code = LD_code | 0x8; //RY
	rd = 0x06;
	mode = 2;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, LD, rd, RY, mode);
}

TEST_F(DecoderDataOps, LD_RY_3) {
	code = LD_code | 0x8; //RY
	rd = 0x19;
	mode = 3;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, LD, rd, RY, mode);
}

TEST_F(DecoderDataOps, LD_RZ_2) {
	code = LD_code; //RZ
	rd = 0x06;
	mode = 2;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, LD, rd, RZ, mode);
}

TEST_F(DecoderDataOps, LD_RZ_3) {
	code = LD_code; //RZ
	rd = 0x19;
	mode = 3;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, LD, rd, RZ, mode);
}

uint16_t LDD_code = 0x8000;

TEST_F(DecoderDataOps, LDD_zero) {
	code = LDD_code;
	rd = 0;
	q = 0;
	code |= encode_rd(rd) | encode_q(q);
	decodeInstruction(&inst, &code);
	EXPECT_INST_Q(inst, LDD, rd, q);
}

TEST_F(DecoderDataOps, LDD_num_1) {
	code = LDD_code;
	rd = 0x1A;
	q = 0x26;
	code |= encode_rd(rd) | encode_q(q);
	decodeInstruction(&inst, &code);
	EXPECT_INST_Q(inst, LDD, rd, q);
}

TEST_F(DecoderDataOps, LDD_num_2) {
	code = LDD_code;
	rd = 0x05;
	q = 0x19;
	code |= encode_rd(rd) | encode_q(q);
	decodeInstruction(&inst, &code);
	EXPECT_INST_Q(inst, LDD, rd, q);
}

uint16_t STS_code = 0x9200;

TEST_F(DecoderDataOps, STS_zero) {
	uint16_t code[2];
	code[0] = STS_code;
	rd = 0;
	al = 0x0000;
	code[1] = al;
	code[0] |= encode_rd(rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_ONE_AL(inst, STS, rd, al);
}

TEST_F(DecoderDataOps, STS_num_1) {
	uint16_t code[2];
	code[0] = STS_code;
	rd = 0xA;
	al = 0xA596;
	code[1] = al;
	code[0] |= encode_rd(rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_ONE_AL(inst, STS, rd, al);
}

TEST_F(DecoderDataOps, STS_num_2) {
	uint16_t code[2];
	code[0] = STS_code;
	rd = 0x15;
	al = 0x5A69;
	code[1] = al;
	code[0] |= encode_rd(rd);
	decodeInstruction(&inst, code);
	EXPECT_INST_ONE_AL(inst, STS, rd, al);
}

uint16_t ST_code = 0x9200;

TEST_F(DecoderDataOps, ST_RX_1) {
	code = ST_code | 0xC; //RX
	rd = 0;
	mode = 1;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, ST, rd, RX, mode);
}

TEST_F(DecoderDataOps, ST_RX_2) {
	code = ST_code | 0xC; //RX
	rd = 0x06;
	mode = 2;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, ST, rd, RX, mode);
}

TEST_F(DecoderDataOps, ST_RX_3) {
	code = ST_code | 0xC; //RX
	rd = 0x19;
	mode = 3;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, ST, rd, RX, mode);
}

TEST_F(DecoderDataOps, ST_RY_2) {
	code = ST_code | 0x8; //RY
	rd = 0x06;
	mode = 2;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, ST, rd, RY, mode);
}

TEST_F(DecoderDataOps, ST_RY_3) {
	code = ST_code | 0x8; //RY
	rd = 0x19;
	mode = 3;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, ST, rd, RY, mode);
}

TEST_F(DecoderDataOps, ST_RZ_2) {
	code = ST_code; //RZ
	rd = 0x06;
	mode = 2;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, ST, rd, RZ, mode);
}

TEST_F(DecoderDataOps, ST_RZ_3) {
	code = ST_code; //RZ
	rd = 0x19;
	mode = 3;
	code |= encode_mode(mode) | encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_DATA(inst, ST, rd, RZ, mode);
}

uint16_t STD_code = 0x8200;

TEST_F(DecoderDataOps, STD_zero) {
	code = STD_code;
	rd = 0;
	q = 0;
	code |= encode_rd(rd) | encode_q(q);
	decodeInstruction(&inst, &code);
	EXPECT_INST_Q(inst, STD, rd, q);
}

TEST_F(DecoderDataOps, STD_num_1) {
	code = STD_code;
	rd = 0x1A;
	q = 0x26;
	code |= encode_rd(rd) | encode_q(q);
	decodeInstruction(&inst, &code);
	EXPECT_INST_Q(inst, STD, rd, q);
}

TEST_F(DecoderDataOps, STD_num_2) {
	code = STD_code;
	rd = 0x05;
	q = 0x19;
	code |= encode_rd(rd) | encode_q(q);
	decodeInstruction(&inst, &code);
	EXPECT_INST_Q(inst, STD, rd, q);
}

uint16_t LPM_code = 0x95C8;

TEST_F(DecoderDataOps, LPM_1) {
	code = LPM_code;
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, LPM);
	EXPECT_EQ(1, inst.mode);
}

uint16_t LPM_EX_code = 0x9004;

TEST_F(DecoderDataOps, LPM2_num_zero) {
	code = LPM_EX_code; // Mode ii
	rd = 0;
	mode = 2;
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_MODE(inst, LPM, rd, mode);
}

TEST_F(DecoderDataOps, LPM2_num_1) {
	code = LPM_EX_code; // Mode ii
	rd = 0x19;
	mode = 2;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_MODE(inst, LPM, rd, mode);
}

TEST_F(DecoderDataOps, LPM2_num_2) {
	code = LPM_EX_code; // Mode ii
	rd = 0x06;
	mode = 2;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_MODE(inst, LPM, rd, mode);
}

TEST_F(DecoderDataOps, LPM3_num_1) {
	code = LPM_EX_code | 0x1; // Mode iii
	rd = 0x19;
	mode = 3;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_MODE(inst, LPM, rd, mode);
}

TEST_F(DecoderDataOps, LPM3_num_2) {
	code = LPM_EX_code | 0x1; // Mode iii
	rd = 0x06;
	mode = 3;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_MODE(inst, LPM, rd, mode);
}

uint16_t ELPM_code = 0x95D8;

TEST_F(DecoderDataOps, ELPM_1) {
	code = ELPM_code;
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, ELPM);
	EXPECT_EQ(1, inst.mode);
}

uint16_t ELPM_EX_code = 0x9004;

TEST_F(DecoderDataOps, ELPM2_num_zero) {
	code = ELPM_EX_code | 0x2; // Mode ii
	rd = 0;
	mode = 2;
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_MODE(inst, ELPM, rd, mode);
}

TEST_F(DecoderDataOps, ELPM2_num_1) {
	code = ELPM_EX_code | 0x2; // Mode ii
	rd = 0x19;
	mode = 2;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_MODE(inst, ELPM, rd, mode);
}

TEST_F(DecoderDataOps, ELPM2_num_2) {
	code = LPM_EX_code | 0x2; // Mode ii
	rd = 0x06;
	mode = 2;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_MODE(inst, ELPM, rd, mode);
}

TEST_F(DecoderDataOps, ELPM3_num_1) {
	code = LPM_EX_code | 0x3; // Mode iii
	rd = 0x19;
	mode = 3;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_MODE(inst, ELPM, rd, mode);
}

TEST_F(DecoderDataOps, ELPM3_num_2) {
	code = LPM_EX_code | 0x3; // Mode iii
	rd = 0x06;
	mode = 3;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_MODE(inst, ELPM, rd, mode);
}

uint16_t SPM_code = 0x9508;

TEST_F(DecoderDataOps, SPM_1) {
	code = SPM_code | 0xE0;	// Mode i-iii
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, SPM);
	EXPECT_EQ(1, inst.mode);
}

TEST_F(DecoderDataOps, SPM_2) {
	code = SPM_code | 0xF0;	// Mode iv-vi
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, SPM);
	EXPECT_EQ(2, inst.mode);
}

uint16_t IN_code = 0xB000;

TEST_F(DecoderDataOps, IN_zero) {
	code = IN_code;
	rd = 0;
	a = 0;
	code |= encode_rd(rd) | (a & 0xF) | ((a & 0x30) << 5);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_A(inst, IN, rd, a);
}

TEST_F(DecoderDataOps, IN_num1) {
	code = IN_code;
	rd = 0x1A;
	a = 0x19;
	code |= encode_rd(rd) | (a & 0xF) | ((a & 0x30) << 5);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_A(inst, IN, rd, a);
}

TEST_F(DecoderDataOps, IN_num2) {
	code = IN_code;
	rd = 0x05;
	a = 026;
	code |= encode_rd(rd) | (a & 0xF) | ((a & 0x30) << 5);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_A(inst, IN, rd, a);
}

uint16_t OUT_code = 0xB800;

TEST_F(DecoderDataOps, OUT_zero) {
	code = OUT_code;
	rd = 0;
	a = 0;
	code |= encode_rd(rd) | (a & 0xF) | ((a & 0x30) << 5);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_A(inst, OUT, rd, a);
}

TEST_F(DecoderDataOps, OUT_num1) {
	code = OUT_code;
	rd = 0x1A;
	a = 0x19;
	code |= encode_rd(rd) | (a & 0xF) | ((a & 0x30) << 5);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_A(inst, OUT, rd, a);
}

TEST_F(DecoderDataOps, OUT_num2) {
	code = OUT_code;
	rd = 0x05;
	a = 026;
	code |= encode_rd(rd) | (a & 0xF) | ((a & 0x30) << 5);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE_A(inst, OUT, rd, a);
}

uint16_t PUSH_code = 0x920F;

TEST_F(DecoderDataOps, PUSH_zero) {
	code = PUSH_code;
	rd = 0;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, PUSH, rd);
}

TEST_F(DecoderDataOps, PUSH_num_1) {
	code = PUSH_code;
	rd = 0x1A;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, PUSH, rd);
}

TEST_F(DecoderDataOps, PUSH_num_2) {
	code = PUSH_code;
	rd = 0x05;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, PUSH, rd);
}

uint16_t POP_code = 0x900F;

TEST_F(DecoderDataOps, POP_zero) {
	code = POP_code;
	rd = 0;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, POP, rd);
}

TEST_F(DecoderDataOps, POP_num_1) {
	code = POP_code;
	rd = 0x1A;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, POP, rd);
}

TEST_F(DecoderDataOps, POP_num_2) {
	code = POP_code;
	rd = 0x05;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, POP, rd);
}

#ifdef XMEGA_SUPPORTED
TEST(DecoderDataOps, XCH_op) {
	ASSERT_TRUE(false) << "XCH test not written";
}

TEST(DecoderDataOps, LAS_op) {
	ASSERT_TRUE(false) << "LAS test not written";
}

TEST(DecoderDataOps, LAC_op) {
	ASSERT_TRUE(false) << "LAS test not written";
}

TEST(DecoderDataOps, LAT_op) {
	ASSERT_TRUE(false) << "LAT test not written";
}
#endif // XMEGA_SUPPORTED

////////////////////
// Bit and Bit-test
////////////////////

uint16_t LSR_code = 0x9406;

TEST_F(DecoderOneOps, LSR_num) {
	code = LSR_code;
	rd = 11;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, LSR, rd);
}

uint16_t ROR_code = 0x9407;

TEST_F(DecoderOneOps, ROR_num) {
	code = ROR_code;
	rd = 18;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, ROR, rd);
}

uint16_t ASR_code = 0x9405;

TEST_F(DecoderOneOps, ASR_num) {
	code = ASR_code;
	rd = 23;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, ASR, rd);
}

uint16_t SWAP_code = 0x9402;

TEST_F(DecoderOneOps, SWAP_num) {
	code = SWAP_code;
	rd = 23;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, SWAP, rd);
}

uint16_t SEx_code = 0x9408;

TEST_F(DecoderOneOps, SEx_zero) {
	code = SEx_code;
	rd = 0;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, SEx, rd);
}

TEST_F(DecoderOneOps, SEx_num_1) {
	code = SEx_code;
	rd = 5;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, SEx, rd);
}

TEST_F(DecoderOneOps, SEx_num_2) {
	code = SEx_code;
	rd = 2;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_ONE(inst, SEx, rd);
}

uint16_t CLx_code = 0x9488;

TEST_F(DecoderOneOps, CLx_zero) {
	code = CLx_code;
	rd = 0;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, CLx);
	EXPECT_EQ(rd, inst.R);
}

TEST_F(DecoderOneOps, CLx_num_1) {
	code = CLx_code;
	rd = 5;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, CLx);
	EXPECT_EQ(rd, inst.R);
}

TEST_F(DecoderOneOps, CLx_num_2) {
	code = CLx_code;
	rd = 2;
	code |= encode_rd(rd);
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, CLx);
	EXPECT_EQ(rd, inst.R);
}


////////////////////////////
// MCU Control Instructions
////////////////////////////

uint16_t BREAK_code = 0x9598;

TEST_F(DecoderF, BREAK_op) {
	code = BREAK_code;
	decodeInstruction(&inst, &code);
	EXPECT_INST_BASIC(inst, BREAK);
}
