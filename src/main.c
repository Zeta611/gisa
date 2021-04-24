#include "../build/src/parser.tab.h"
#include <stdbool.h>
#include <stdio.h>

char *progname;
int lineno = 1;
bool fin = false;
extern FILE *yyin;

int main(int argc, char *argv[])
{
	progname = argv[0];

	// Handle input file.
	if (argc > 1) {
		fin = true;
		yyin = fopen(argv[1], "r");
	}

	yyparse();

	if (fin) {
		fclose(yyin);
	}
	return 0;
}
