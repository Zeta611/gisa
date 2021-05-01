#include "ast.h"
#include "eval.h"
#include "parser.tab.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *progname;
int lineno = 1;
extern FILE *yyin;

int main(int argc, char *argv[])
{
	progname = argv[0];

	// Input file name
	char *fin = NULL;
	// Handle input file.
	if (argc > 1) {
		fin = argv[1];
		errno = 0;
		if (!(yyin = fopen(argv[1], "r"))) {
			fprintf(stderr, "%s: cannot access '%s': %s\n",
				progname, fin, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	ASTNode *nlist = NULL; // owns all allocated `ASTNode`s.
	ASTNode *ast = NULL;
	if (!yyparse(&nlist, &ast)) {
		p_sexp_ast(ast);
		putchar('\n');

		srand(time(NULL));
		Env env = {0., 0.};
		eval(ast, &env);
		printf("x: %lf, y: %lf\n", env.x, env.y);
	}
	free_nodes(nlist);

	if (fin) {
		if (fclose(yyin)) {
			fprintf(stderr, "%s: fclose error '%s': %s\n", progname,
				fin, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}
