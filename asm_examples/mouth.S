#ifndef __AVR_ATmega328__
	#define __AVR_ATmega328
#endif

#include <avr/io.h>

.global main

main:
	CLR R0
	LDI r31, 0x02
	LDI r30, 0x00
LOOP:
	BREAK
	ST Z, R0
	INC R0
	RJMP LOOP
