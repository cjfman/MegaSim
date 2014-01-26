// opcode_defs.h
//
// Developed by Charles Franklin
//
// MIT Lisense

#ifndef OPCODE_DEFS_h
#define OPCODE_DEFS_h
//#define XMEGA_SUPPORTED

// Core levels
#define CORE_L_MIN	 89 // Minimum Core
#define CORE_L_8k	 99 // Clasic Cores
#define CORE_L_128k	102
#define CORE_L_E8k	111 // Enhanced Cores
#define CORE_L_E128k	112
#define CORE_L_E4M	114
#define CORE_L_XMEGA	119  // Mega core
#define CORE_L_REDUC	  0  // Reduced // Unsupported for now

// Opcode Definitions
// Basic Core
// Arithmetic
#define ADD	33	// ADD was 0, but I need that number for invalid OP (NULL)
#define ADC	1
#define SUB	2
#define SUBI	3
#define SBC	4
#define SBCI	5
#define AND	6
#define ANDI	7
#define OR	8
#define ORI	9
#define EOR	10
#define COM	11
#define NEG	12
#define SBR	13
#define CBR	14
#define INC	15
#define DEC	16
#define TST	17	// Secretly AND
#define CLR	18	// Secretly EOR
#define SER	19
// Branches
#define RJMP	20
#define RCALL	21
#define RET	22
#define RETI	23
#define CPSE	24
#define CP	25
#define CPC	26
#define CPI	27
#define SBRC	28
#define SBRS	29
#define SBIC	30
#define SBIS	31
#define BRx	32	// Covers all SREG conditional branches
#define BRBS	32	// Covered by the specific conditional branches
#define BRBC	33	// This one too. ADD borrows 33 for it's code.
#define BREQ	34
#define BRNE	35
#define BRCS	36
#define BRCC	37
#define BRSH	38	// aka BRCC
#define BRLO	39
#define BRMI	40
#define BRPL	41
#define BRGE	42
#define BRLT	43
#define BRHS	44
#define BRHC	45
#define BRTS	46
#define BRTC	47
#define BRVS	48
#define BRVC	49
#define BRIE	50
#define BRID	51
// Transfers
#define LD	52
#define ST	53
#define MOV	54
#define LDI	55
#define IN	56
#define OUT	57
#define LPM	58
// Bitwise
#define SBI	59
#define CBI	60
#define LSL	61	// Secretly ADD
#define LSR	62
#define ROL	63	// Secretly ADC
#define ROR	64
#define ASR	65
#define SWAP	66
#define BSET	67	// Covered by SEx
#define BCLR	68	// Covered by CLx
#define BST	69
#define BLD	70
#define SEx	71	// The set and clear operations
#define CLx     72	// are decoded differently. They
#define SEC	71	// Are decoded as SEx and CLx.
#define CLC	72
#define SEN	73	// The bits in the status register
#define CLN	74	// Are indexed in this order
#define SEZ	75	// +-------------------------------+
#define CLZ	76	// | I | T | H | S | V | N | Z | C |
#define SEI	77	// +-------------------------------+
#define CLI	78	//  MSB                         LSB
#define SES	79
#define CLS	80
#define SEV	81
#define CLV	82
#define SET	83
#define CLT	84
#define SEH	85
#define CLH	86
#define NOP	87
#define SLEEP	88
#define WDR	89
// Classic Core 8k
// Arithmetic 
#define ADIW	90
#define SBIW	91
// Braching
#define IJMP	92
#define ICALL	93
// Transfer
#define LDD	94
#define LDS	95
#define STD	96
#define STS	97
#define PUSH	98	// Secretly ST with mode 7
#define POP	99	// Secretly LD with mode 7
// Classic Core 128k
// Branching
#define JMP	100
#define CALL	101
// Transfer
#define ELPM	102
// Enhanced Core 8k
// Arithmetic 
#define MUL	103
#define MULS	104
#define MULSU	105
#define FMUL	106
#define FMULS	107
#define FMULSU	108
// Transfer
#define MOVW	109
#define LPM	110
#define SPM	111
// Enhanced Core 128k
// Bitwise
#define BREAK	112
// Enhanced Core 4M
// Branching
#define EIJMP	113
#define EICALL	114
// XMEGA Core
// Arithmetic
#define DES	115
// Transfer
#define XCH	116
#define LAS	117
#define LAC	118
#define LAT	119

#endif
