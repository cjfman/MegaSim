%{

#include <stdlib.h>
#include <stdio.h>
#include "debugterm.tab.h"

%}

%option noyywrap 

%%

 /* White space */
[ \n\t] ;

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

 /* end sequence */
[;]						{ 
							return 0;
						}

 /* Everything Else */
. 						{ return yytext[0]; }

%%

/*
int main() {
	yylex();
}
*/
