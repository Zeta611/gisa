%code {
#include <stdio.h>
}

%code requires {
#include "ast.h"
}

%code provides {
// `SS` must not have any side-effect. `SS` is intended to be substituted with
// yacc's `$$`.
#define CHK_NULL_NODE(SS, NODE)                                                \
	do {                                                                   \
		if (!((SS) = (NODE))) {                                        \
			fprintf(stderr, "Failed to allocate memory.\n");       \
			YYABORT;                                               \
		}                                                              \
	} while (0)

int yyerror(ASTNode **nlist, ASTNode **ast, const char *msg);
}

%start	hook

%union {
	double	num;
	Var	var;
	ASTNode	*node;
}

%token		OR INIT ITER TRANSLATION ROTATION ERR
%token	<num>	NUM
%token	<var>	VAR

%type	<node>	prgm init translation rotation sequence or iter
		block region interval
		poly mult neg expt atom

%right	';'

%parse-param { ASTNode **nlist } { ASTNode **ast }

%%
hook:	  prgm	{ *ast = $1; }
	;
prgm:	  init
	| translation
	| rotation
	| sequence
	| or
	| iter
	;
init:	  INIT '(' region ')'	{ CHK_NULL_NODE($$, init_node(nlist, $3)); }
	;
translation:	  TRANSLATION '(' poly ',' poly ')' {
			CHK_NULL_NODE($$, translation_node(nlist, $3, $5)); }
		;
rotation:	  ROTATION '(' poly ',' poly ',' poly ')' {
			CHK_NULL_NODE($$, rotation_node(nlist, $3, $5, $7)); }
		;
sequence:	  prgm ';' prgm	{
			CHK_NULL_NODE($$, sequence_node(nlist, $1, $3)); }
		;
or:	  block OR block	{ CHK_NULL_NODE($$, or_node(nlist, $1, $3)); }
	;
iter:	  ITER block	{ CHK_NULL_NODE($$, iter_node(nlist, $2)); }
	;

block:	  '{' prgm '}'	{ CHK_NULL_NODE($$, $2); }
	;
region:	  interval '*' interval	{
		CHK_NULL_NODE($$, region_node(nlist, $1, $3)); }
	/* | relations	{ */
	/*         CHK_NULL_NODE($$, region_node(nlist, $1, $3)); } */
	;
interval:	  '[' poly ',' poly ']'	{
			CHK_NULL_NODE($$, interval_node(nlist, $2, $4)); }
		;

poly:	  mult
	| poly '+' mult	{ CHK_NULL_NODE($$, op_node(nlist, ADD, $1, $3)); }
	| poly '-' mult	{ CHK_NULL_NODE($$, op_node(nlist, SUB, $1, $3)); }
	;
mult:	  neg
	| mult '*' neg	{ CHK_NULL_NODE($$, op_node(nlist, MUL, $1, $3)); }
	| mult '/' neg	{ CHK_NULL_NODE($$, op_node(nlist, DIV, $1, $3)); }
	| mult expt	{ CHK_NULL_NODE($$, op_node(nlist, MUL, $1, $2)); }
	;
neg:	  expt
	| '-' neg	{ CHK_NULL_NODE($$, op_node(nlist, NEG, $2, NULL)); }
	;
expt:	  atom
	| atom '^' neg	{ CHK_NULL_NODE($$, op_node(nlist, POW, $1, $3)); }
	;
atom:	  NUM	{ CHK_NULL_NODE($$, num_node(nlist, $1)); }
	| VAR	{ CHK_NULL_NODE($$, var_node(nlist, $1)); }
	| '(' poly ')'	{ $$ = $2; }
	| ERR	{ yyerror(nlist, ast, "syntax error"); YYABORT; }
	;
%%

extern int lineno;
extern char *progname;
int yyerror(ASTNode **nlist, ASTNode **ast, const char *msg)
{
	(void)nlist;
	(void)ast;
	fprintf(stderr, "%s: %s near line %d\n", progname, msg, lineno);
	return 0;
}
