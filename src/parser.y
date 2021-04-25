%{
#include "ast.h"
#include <stdio.h>
%}

%start	prgm

%union {
	long long num;
	ASTNode *node;
}

%token		OR INIT ITER TRANSLATION ROTATION
%token	<num>	NUM

%type	<node>	prgm init translation rotation block region interval atom

%parse-param { ASTNode **ast }

%%

prgm:	  init
	| translation
	| rotation
	| prgm ';' prgm	{ $$ = *ast = sequence_node($1, $3); }
	| block OR block	{ $$ = *ast = or_node($1, $3); }
	| ITER block	{ $$ = *ast = iter_node($2); }
	;
block:	  '{' prgm '}'	{ $$ = *ast = $2; }
	;
init:	  INIT '(' region ')'	{ $$ = *ast = init_node($3); }
	;
region:	  interval '*' interval	{ $$ = *ast = region_node($1, $3); }
	;
interval:	  '[' atom ',' atom ']' { $$ = *ast = interval_node($2, $4); }
		;
translation:	  TRANSLATION '(' atom ',' atom ')' {
			$$ = *ast = translation_node($3, $5); }
		;
rotation:	  ROTATION '(' atom ',' atom ',' atom ')' {
			$$ = *ast = rotation_node($3, $5, $7); }
		;
atom:	  NUM	{ $$ = *ast = num_node($1); }
	;
