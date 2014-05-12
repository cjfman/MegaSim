#ifndef __AVR_ATmega328__
	#define __AVR_ATmega328
#endif

#include <avr/io.h>

.global main

main:
	CLR r0
LOOP:
	INC r0
	RJMP LOOP
