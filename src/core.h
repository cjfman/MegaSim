// Core.h

// Developed by Charles Franklin
//
// MIT Lisense

#ifndef CORE_h
#define CORE_h

#include <inttypes.h>
#include "devices.h"

#define IS_ARITHMETIC_RS (-2 >> 1 == -1)

// MACROS
#define GETBIT(var, index) ((var >> index) 0x01)

////////////////////////
// Memory Structures
////////////////////////

// Status register
// +-------------------------------+
// | I | T | H | S | V | N | Z | C |
// +-------------------------------+
//  MSB                         LSB
#define CREG 0 // C: Carry Flag
#define ZREG 1 // Z: Zero Flag
#define NREG 2 // N: Negative Flag
#define VREG 3 // V: Two's complement overflow indicator
#define SREG 4 // S: N XOR V, for signed tests
#define HREG 5 // H: half Carry Flag
#define TREG 6 // T: Tranfer bit
#define IREG 7 // I: Global Interrupt Enable/Disable Flag
bool sreg[8];

// General Perpose Registers
uint8_t regs[32];
uint16_t *regps = (uint16_t*)regs; // Register pairs

// Program Counter
uint32_t pc;

// Main Memory
uint8_t *main_mem;

// I/O Memory
uint8_t *io_mem;

// Program Memory
typedef struct Program {
    uint16_t *data;
    int size;
} Program;

Program *prog_mem;

// Indirect Registers and Poiners
uint16_t *RW = (uint16_t*)(&regs[24]);	// R25:R24
uint16_t *RX = (uint16_t*)(&regs[26]);	// R27:R26
uint16_t *RY = (uint16_t*)(&regs[28]);	// R29:R28
uint16_t *RZ = (uint16_t*)(&regs[30]);	// R31:R30
uint8_t *EIND;
uint32_t *SP;

////////////////////////
// Device Properties
////////////////////////

coreType core;

////////////////////////
// Error Codes and Vars
////////////////////////

#define MEM_ERROR 0x01
#define PROG_MEM_ERROR 0x02

int error_val;

////////////////////////
// Opt Code Decoding
////////////////////////

// Stores decoded instructions
typedef struct Instuction {
	uint8_t op;		// The basic operation
	uint8_t R;		// The source register. Sometimes SREG bit index. 
	uint8_t D;		// The destination/source register. Sometimes bit value.
	uint8_t mode;	// Mode of the operation
	uint16_t A;		// Address and offset
	int8_t K;		// Constant
	uint16_t *ireg;	// Indirect register
	uint8_t wsize;	// The size in words of the instruction
} Instruction;

// Decodes instructions
decodedInstruction(Instruction *inst, uint16_t opcode);


//////////////////////////////////
// Instruction Implementations
//////////////////////////////////

int ADC_run(Instruction *inst);
int ADD_run(Instruction *inst);
int ADIW_run(Instruction *inst);
int AND_run(Instruction *inst);
int ANDI_run(Instruction *inst);
int ASR_run(Instruction *inst);
//int BCLR_run(Instruction *inst);
int BLD_run(Instruction *inst);
int BRx_run(Instruction *inst);
//int BRBC_run(Instruction *inst);
//int BRBS_run(Instruction *inst);
//int BRCC_run(Instruction *inst);
//int BRCS_run(Instruction *inst);
int BREAK_run(Instruction *inst);
//int BREQ_run(Instruction *inst);
//int BRGE_run(Instruction *inst);
//int BRHC_run(Instruction *inst);
//int BRHS_run(Instruction *inst);
//int BRID_run(Instruction *inst);
//int BRIE_run(Instruction *inst);
//int BRLO_run(Instruction *inst);
//int BRLT_run(Instruction *inst);
//int BRMI_run(Instruction *inst);
//int BRNE_run(Instruction *inst);
//int BRPL_run(Instruction *inst);
//int BRSH_run(Instruction *inst);
//int BRTC_run(Instruction *inst);
//int BRTS_run(Instruction *inst);
//int BRVC_run(Instruction *inst);
//int BRVS_run(Instruction *inst);
//int BSET_run(Instruction *inst);
int BST_run(Instruction *inst);
int CALL_run(Instruction *inst);
int CBI_run(Instruction *inst);
int CLx_run(Instruction *inst);
//int CBR_run(Instruction *inst);
//int CLC_run(Instruction *inst);
//int CLH_run(Instruction *inst);
//int CLI_run(Instruction *inst);
//int CLN_run(Instruction *inst);
//int CLR_run(Instruction *inst);
//int CLS_run(Instruction *inst);
//int CLT_run(Instruction *inst);
//int CLV_run(Instruction *inst);
//int CLZ_run(Instruction *inst);
int COM_run(Instruction *inst);
int CP_run(Instruction *inst);
int CPC_run(Instruction *inst);
int CPI_run(Instruction *inst);
int CPSE_run(Instruction *inst);
int DEC_run(Instruction *inst);
#ifdef XMEGA_SUPPORTED
int DES_run(Instruction *inst);
#endif
int EICALL_run(Instruction *inst);
int EIJMP_run(Instruction *inst);
int ELPM_run(Instruction *inst);
int EOR_run(Instruction *inst);
int FMUL_run(Instruction *inst);
int FMULS_run(Instruction *inst);
int FMULSU_run(Instruction *inst);
int ICALL_run(Instruction *inst);
int IJMP_run(Instruction *inst);
int IN_run(Instruction *inst);
int INC_run(Instruction *inst);
int JMP_run(Instruction *inst);
int LAC_run(Instruction *inst);
int LAS_run(Instruction *inst);
int LAT_run(Instruction *inst);
int LD_run(Instruction *inst);
int LDD_run(Instruction *inst);
int LDI_run(Instruction *inst);
int LDS_run(Instruction *inst);
int LPM_run(Instruction *inst);
int LSL_run(Instruction *inst);
int LSR_run(Instruction *inst);
int MOV_run(Instruction *inst);
int MOVW_run(Instruction *inst);
int MUL_run(Instruction *inst);
int MULS_run(Instruction *inst);
int MULSU_run(Instruction *inst);
int NEG_run(Instruction *inst);
int NOP_run(Instruction *inst);
int OR_run(Instruction *inst);
int ORI_run(Instruction *inst);
int OUT_run(Instruction *inst);
int POP_run(Instruction *inst);
int PUSH_run(Instruction *inst);
int RCALL_run(Instruction *inst);
int RET_run(Instruction *inst);
int RETI_run(Instruction *inst);
int RJMP_run(Instruction *inst);
int ROL_run(Instruction *inst);
int ROR_run(Instruction *inst);
int SBC_run(Instruction *inst);
int SBCI_run(Instruction *inst);
int SBI_run(Instruction *inst);
int SBIC_run(Instruction *inst);
int SBIS_run(Instruction *inst);
int SBIW_run(Instruction *inst);
//int SBR_run(Instruction *inst);
int SBRC_run(Instruction *inst);
int SBRS_run(Instruction *inst);
//int SEC_run(Instruction *inst);
//int SEH_run(Instruction *inst);
//int SEI_run(Instruction *inst);
//int SEN_run(Instruction *inst);
//int SER_run(Instruction *inst);
//int SES_run(Instruction *inst);
//int SET_run(Instruction *inst);
//int SEV_run(Instruction *inst);
int SEx_run(Instruction *inst);
int SEZ_run(Instruction *inst);
int SLEEP_run(Instruction *inst);
int SPM_run(Instruction *inst);
int ST_run(Instruction *inst);
int STD_run(Instruction *inst);
int STS_run(Instruction *inst);
int SUB_run(Instruction *inst);
int SUBI_run(Instruction *inst);
int SWAP_run(Instruction *inst);
//int TST_run(Instruction *inst);
int WDR_run(Instruction *inst);
int XCH_run(Instruction *inst);

/////////////////////////////////////
// Error Codes
/////////////////////////////////////

#define BAD_REG 1

#endif
