#ifndef __AVR_ATmega328__
	#define __AVR_ATmega328
#endif

#include <avr/io.h>

.global main

main:
	RCALL print
	.ascii "Hello World!\n\0"
	BREAK
	RJMP main

; print
print:
	; Push used registers
	PUSH 16
	PUSH R28	; LY
	PUSH R29	; HY
	PUSH R30	; LZ
	PUSH R31	; HZ
	
	; Get address of string
	LDS R28, SPL
	LDS R29, SPH
	LDD  R30, Y+6
	LDD R31, Y+7
	; Load Y with stderr address
	LDI R28, 0xE0
	CLR R29
	; Correct for byte addressing
	LSL R30
	ROL R31
print_1:
	LPM R16, Z+
	CPI R16, 0
	BREQ print_end
	ST Y, R16
	;STS $0x00E0, R16
	RJMP print_1
print_end:
	; Correct return address
	CLR R1
	LSR R31
	ROR R30
	LDS R28, SPL
	LDS R29, SPH
	STD Y+6, R30
	STD Y+7, R31
	; POP registers
	POP R31	; HZ
	POP R30	; LZ
	POP R29	; HY
	POP R28	; LY
	POP R16
	RET

