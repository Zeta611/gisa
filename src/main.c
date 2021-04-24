#include "../build/src/parser.tab.h"
#include <stdio.h>

char *progname;
int lineno = 1;

int main(int argc, char *argv[])
{
	yyparse();
	return 0;
}
