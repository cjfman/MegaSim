// debugterm.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "debugterm.h"
#include "core.h"
#include "handlers.h"
#include "decoder.h"

void yyparse(void);

int debug_mode = 0;

/*
int runDebugTerm(void) {
	fprintf(stderr, "\nStarting Debug Terminal\n");
	debug_mode = 1;
	// Print next instruction
	Instruction inst = program->instructions[pc];
	fprintf(stderr, "pc @ 0x%X\t", pc);
	printInstruction(&inst);
	fprintf(stderr, "\n");
	// Get commands
	int i, error;
	i = 0;
	char *c = (char*)malloc(1024);;
	while(1) {
		fprintf(stderr, ">>");
		scanf("%1024s", c);
		if (strcmp(c, "continue") == 0) 
			break;
		else if (strcmp(c, "step") == 0) {
			error = step();
			if (error) return error;
			continue;
		}
		else if (strcmp(c, "disasm") == 0) {
			printDisAsm();
			continue;
		}
		else if (strcmp(c, "quit") == 0) {
			return EXIT_ERROR;
		}
	}
	debug_mode = 0;
	return 0;
}
// */

int step(void) {
	Instruction inst = program->instructions[pc];
	int error = handlers[inst.op](&inst);
	if (error < 0) {
		return error;
	}   
	if (pc >= program->size) {
		return PC_ERROR;
	} 
	fprintf(stderr, "pc @ 0x%X\t", pc);
	printInstruction(&program->instructions[pc]);
	fprintf(stderr, "\n");
	return 0;
}

