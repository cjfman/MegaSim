// handlers.h

// Developed by Charles Franklin
//
// MIT Lisense

#ifndef HANDLERS_h
#define HANDLERS_h

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include "bool.h"
#include "core.h"
#include "decoder.h"

// MACROS
#define GETBIT(var, index) ((var >> index) 0x01)

////////////////////////
// Error Codes and Vars
////////////////////////

#define MEM_ERROR 0x01
#define PROG_MEM_ERROR 0x02

int error_val;

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

#ifdef __cplusplus
}
#endif

#endif // HANDLERS_h
