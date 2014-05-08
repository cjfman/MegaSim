%{

#include <stdlib.h>
#include <stdio.h>
#include "debugterm.tab.h"

#ifdef __cplusplus
extern "C" {
#endif	// __cplusplus

%}

%option noyywrap 

ALPHA_L = [a-z]
ALPHA_H = [A-Z]
ALPHA = ALPHA_L|ALPHA_H

%%

 /* White space */
[ \t] ;

 /* Constants */
(0x[0-9a-fA-F]+)|[0-9]+			{
							//printf("Found constant %d\n", strtol(yytext, 0, 0));
							yylval.ival = strtol(yytext, 0, 0); 
							return CONSTANT; 
						}
 /* Registers */
[Rr][0-9]+              { 
							//printf("Found Register %s\n", yytext);
							yylval.ival = strtol(yytext + 1, 0, 0); 
							return REGISTER; 
						}
 /* Addresses */
 /*
\*[0-9]+				{
							//printf("Found address %s\n", yytext);
							yylval.ival = strtol(yytext + 1, 0, 0); 
							return ADDRESS; 
						}
*/

 /* Commands */
p\/x					{
 							yylval.sval = strdup(yytext);
							return PRINT_HEX;
						}

continue				{ return CONTINUE; }

step					{ return STEP; }

disasm					{ return DISASM; }

quit					{ return QUIT; }

 /* end sequence */
[\n]					{ 
							return 0;
						}

 /* Everything Else */
. 						{ return yytext[0]; }

%%

#ifdef __cplusplus
}
#endif	// __cplusplus
