%{
#include <stdio.h>
#define PRINT(s) fputs(s, stdout)
%}

%define api.value.type { long }

%start	prgm

%token	OR INIT ITER TRANSLATION ROTATION NUM

%%

prgm:	  init	{ putchar('\n'); }
	| init ';' { puts(";"); } p { putchar('\n'); }
	;
p:	  translation
	| rotation
	| p ';' { puts(";"); } p
	| block OR { PRINT(" OR "); } block
	| ITER { PRINT("ITER "); } block
	;
block:	  '{' { puts("{"); } p '}' { puts("\n}"); }
	;
init:	  INIT '(' { PRINT("INIT("); } region ')' { putchar (')'); }
	;
region:	  interval '*' { PRINT(" * "); } interval
	;
interval:	  '[' NUM ',' NUM ']' {
			printf("INTERVAL(%ld, %ld)", $2, $4);
		}
		;
translation:	  TRANSLATION '(' NUM ',' NUM ')' {
			printf("TRANSLATION(%ld, %ld)", $3, $5);
		}
		;
rotation:	  ROTATION '(' NUM ',' NUM ',' NUM ')' {
			printf("ROTATE(%ld, %ld, %ld)", $3, $5, $7);
		}
		;
%%

char *progname;
int lineno = 1;

int main(int argc, char *argv[])
{
	yyparse();
	return 0;
}
