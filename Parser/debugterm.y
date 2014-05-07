%{
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "debugterm.h"
#include "core.h"
#include "handlers.h"
#include "decoder.h"

#define YYDEBUG 1
#define YYERROR_VERBOSE 1

#ifdef __cplusplus
extern "C" {
#endif

void yyerror(const char *s);
int yylex(void);

int yydebug;
int handle_command;

%}

%union {
	int ival;
	char cval;
	char *sval;
}


%token <ival> CONSTANT
%token <ival> ADDRESS
%token <ival> REGISTER

// Commands
%token CONTINUE
%token STEP
%token DISASM
%token QUIT
%token PRINT
%token PRINT_HEX

// Types
%type <ival> expression
%type <ival> term
%type <ival> factor
%type <ival> primary
%type <ival> variable

%%

commands: assignment			{ handle_command = 0 }
| expression					{ handle_command = 0 }
| print							{ handle_command = 0 }
| CONTINUE						{ handle_command = CONTINUE }
| STEP							{ handle_command = STEP }
| DISASM						{ handle_command = DISASM }
| QUIT							{ handle_command = QUIT }
;

assignment: variable '=' expression { 
										main_mem[$1] = $3; 
										if ($1 < 32)
											printf("R%d = ", $1);
										else
											printf("MEM[%X] = ", $1);

										printf("%d\n", main_mem[$1]);
									}

print: PRINT_HEX expression		{ printf("0x%X\n", $2); }
| 'p' expression				{ printf("%d\n", $2); }
;

expression: term
| '-' term						{ $$ = -1 * $2; }
| expression '+' term			{ $$ = $1 + $3; }
| expression '-' term			{ $$ = $1 - $3; }	
;

term: factor
| term '*' factor				{ $$ = $1 * $3; }
| term '/' factor				{ 
									if($3 == 0) 
										yyerror("undefined"); 
									else 
										$$ = $1/ $3; 
								}
;

factor: primary					{ $$ = $1; }
;

primary: CONSTANT				{ $$ = $1; }
| variable						{ $$ = main_mem[$1]; }
| '(' expression ')'			{ $$ = $2; }
;

variable: 
	  ADDRESS 					{ $$ = $1; }
	| REGISTER 					{ 
									if ($1 > 31)
										yyerror("Not valid register");
									else
										$$ = $1; 
								}
	;


%%

void yyerror(const char *s) {
	printf("Parse Error! Message: %s\n", s);
}

int runDebugTerm(void) {
	fprintf(stderr, "\nStarting Debug Terminal\n");
	debug_mode = 1;

	// Print next instruction
	Instruction inst = program->instructions[pc];
	fprintf(stderr, "pc @ 0x%X\t", pc);
	printInstruction(&inst);
	fprintf(stderr, "\n");
	while (debug_mode) {
		printf(">");
		int error = yyparse();
		if (error) {
			continue;
		}
		switch (handle_command) {
		case 0: 
			break;
		case QUIT:
			return EXIT_ERROR;
		case STEP:
			error = step();
			if (error) {
				return error;
			}
			break;
		case CONTINUE:
			debug_mode = 0;
			break;
		case DISASM:
			printDisAsm();
			break;
		}
	}
	return 0;
}

#ifdef BISON_COMPILE_MAIN
int main(void) {
	yyparse();
	return 0;
}
#endif // BISON_COMPILE_MAIN

#ifdef __cplusplus
}
#endif
