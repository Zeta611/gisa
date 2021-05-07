#include "ast.h"
#include "eval.h"
#include "parser.tab.h"
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
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
	int ecode = EXIT_SUCCESS;

	// Parse command line arguments
	// Input file name
	char *fin = NULL;
	// Parse mode
	bool show_parse = false;
	// Verbose mode
	bool verbose = false;
	// Maximum iteration
	int iter_max = 300;
	// Seed for `rand`
	unsigned int seed = time(NULL);

	int optidx;
	for (optidx = 1; optidx < argc && argv[optidx][0] == '-'; ++optidx) {
		switch (argv[optidx][1]) {
		case 'p':
			if (argv[optidx][2]) {
				goto invalid_option;
			}
			show_parse = true;
			break;
		case 'v':
			if (argv[optidx][2]) {
				goto invalid_option;
			}
			verbose = true;
			break;
		case 'm': {
			char *num = argv[optidx] + 2;
			if (!*num) {
				// The number is separated from `m`, e.g.,
				// `-m 100`.
				if (!(num = argv[++optidx])) {
					// `-m` is the last argument and not
					// followed by a number.
					fprintf(stderr,
						"%s: missing number for the "
						"flag `-m`\n",
						progname);
					exit(EXIT_FAILURE);
				}
			}
			errno = 0;
			char *end;
			const long lnum = strtol(num, &end, 0);
			if (lnum > INT_MAX || lnum < 0) {
				errno = ERANGE;
			}
			if (errno == ERANGE) {
				fprintf(stderr,
					"%s: number out of range [0, %d] -- "
					"'%s'\n",
					progname, INT_MAX, num);
				exit(EXIT_FAILURE);
			}
			if (*end) { // Invalid character(s) left.
				fprintf(stderr, "%s: invalid number -- '%s'\n",
					progname, num);
				exit(EXIT_FAILURE);
			}
			// Now it is safe to convert.
			iter_max = (int)lnum;
			break;
		}
		case 's': {
			char *num = argv[optidx] + 2;
			if (!*num) {
				// The number is separated from `s`, e.g.,
				// `-s 100`.
				if (!(num = argv[++optidx])) {
					// `-s` is the last argument and not
					// followed by a number.
					fprintf(stderr,
						"%s: missing number for the "
						"flag `-s`\n",
						progname);
					exit(EXIT_FAILURE);
				}
			}
			errno = 0;
			char *end;
			const unsigned long lnum = strtoul(num, &end, 0);
			if (lnum > UINT_MAX) {
				errno = ERANGE;
			}
			if (errno == ERANGE) {
				fprintf(stderr,
					"%s: number out of range [0, %d] -- "
					"'%s'\n",
					progname, UINT_MAX, num);
				exit(EXIT_FAILURE);
			}
			if (*end) { // Invalid character(s) left.
				fprintf(stderr, "%s: invalid number -- '%s'\n",
					progname, num);
				exit(EXIT_FAILURE);
			}
			// Now it is safe to convert.
			seed = (int)lnum;
			break;
		}
		default:
		invalid_option:
			fprintf(stderr,
				"%s: invalid option -- '%s'\n"
				"%s: usage: %s [-p] [-v] [-mITERMAX] [-sSEED] "
				"[FILE]\n",
				progname, argv[optidx], progname, progname);
			exit(EXIT_FAILURE);
		}
	}
	argv += optidx;

	// Handle input file.
	if (*argv) {
		fin = *argv;
		errno = 0;
		if (!(yyin = fopen(*argv, "r"))) {
			fprintf(stderr, "%s: cannot access '%s': %s\n",
				progname, fin, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	// Begin parsing
	ASTNode *nlist = NULL; // owns all allocated `ASTNode`s.
	ASTNode *ast = NULL;
	if (!yyparse(&nlist, &ast)) {
		if (show_parse) {
			// Print the S-expression to `stderr`.
			p_sexp_ast(stderr, ast);
			putc('\n', stderr);
		}

		Env env = {.init = false, .x = 0., .y = 0.};
		srand(seed);
		const int ret = eval(ast, &env, iter_max, verbose);
		switch (ret) {
		case 0:
			printf("(%lf, %lf)\n", env.x, env.y);
			break;
		case 1:
			ecode = false;
			fprintf(stderr,
				"%s: error: operation before initialization\n",
				progname);
			break;
		case 2:
			ecode = false;
			fprintf(stderr, "%s: error: number argument expected\n",
				progname);
			break;
		case 3:
			ecode = false;
			fprintf(stderr,
				"%s: error: polynomial evaluation failed\n",
				progname);
			break;
		default:
			ecode = false;
			fprintf(stderr, "%s: unknown error\n", progname);
			break;
		}
	}

	// Clean up
	free_nodes(nlist);

	if (fin) {
		if (fclose(yyin)) {
			fprintf(stderr, "%s: fclose error '%s': %s\n", progname,
				fin, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	return ecode;
}
