%{
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <time.h>
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
int error_code;

#define UNKNOWN_ERR		0
#define REGISTER_ERR	2
#define ADDRESS_ERR		3

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
%token <sval> COMMAND
%token CONTINUE
%token STEP
%token DISASM
%token QUIT
%token RATE
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
| RATE							{ handle_command = RATE }
| error 						{ handle_command = -1 }
| 								{ handle_command = 0 }
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
									if ($1 > 31) {
										yyerror("Not valid register");
										error_code = REGISTER_ERR;
										YYERROR;
									}
									else
										$$ = $1; 
								}
	;


%%

void yyerror(const char *s) {
	//fprintf(stderr, "Parse Error! Message: %s\n", s);
}

int runDebugTerm(void) {
	uint64_t now_time = time(0);
	debug_mode = 1;
	fprintf(stderr, "\nStarting Debug Terminal\n");

	// Print next instruction
	Instruction inst = program->instructions[pc];
	fprintf(stderr, "pc @ 0x%X\t", pc);
	printInstruction(&inst);
	fprintf(stderr, "\n");
	while (debug_mode) {
		printf(">");
		int error = yyparse();
		if (error) {
			fprintf(stderr, "Unkown error\n");
			error_code = 0;
			continue;
		}
		switch (handle_command) {
		case -1: 
			switch (error_code) {
			case 0:
				fprintf(stderr, "Unknown command\n");
				break;
			case REGISTER_ERR:
				fprintf(stderr, "Bad register\n");
				break;
			case ADDRESS_ERR:
				fprintf(stderr, "Bad address\n");
				break;
			default:
				fprintf(stderr, "Unkown error\n");
				break;
			}
			break;
		case RATE:
		{
			uint64_t diff = now_time - last_time;
			if (diff != 0) {
				uint64_t rate = (cycle_count - last_count) / diff;
				fprintf(stderr, "Rate = %llu instructions/s\n", rate);
			}
			else {
				fprintf(stderr, "Rate error, no time has elapsed\n");
			}
			break;
		}
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
		default:
			break;
		}
	}
	
	// Closing sequence
	last_count = cycle_count;
	last_time = time(0);
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
