%{
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#define YYDEBUG 1
#define YYERROR_VERBOSE 1
//#define YYPRINT(file, type, value)   yyprint (file, type, value)

void yyerror(const char *s);

int yydebug;

uint8_t main_mem[1024];
%}

%union {
	int ival;
	char cval;
	char *sval;
}


%token <ival> CONSTANT
%token <ival> ADDRESS
%token <ival> REGISTER
%token <sval> PRINT_HEX
%type <ival> expression
%type <ival> term
%type <ival> factor
%type <ival> primary
%type <ival> variable

%%

commands: assignment			{ }
| expression					{ }
| print							{ }
;

assignment: variable '=' expression { main_mem[$1] = $3; }

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

int main() {
	yydebug = 1;
	while (1) {
		yyparse();
	}
}

void yyerror(const char *s) {
	printf("Parse Error! Message: %s\n", s);
	exit(-1);
}
