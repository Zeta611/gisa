%{
#include "parser.tab.h"
#include <stdio.h>

extern char *yytext;
extern int lineno;
%}

ws	[ \t]+

digit	[0-9]
int	{digit}+

op	[-+*/^]
par	[\[\](){}]
%%

{ws}	{ ; }	// skip whitespaces
\n	{ ++lineno; }

{int}	{ sscanf(yytext, "%ld", &yylval); return NUM; }

{op}	{ return yytext[0]; }
{par}	{ return yytext[0]; }
\,	{ return yytext[0]; }
\;	{ return yytext[0]; }

"or"	{ return OR; }
"init"	{ return INIT; }
"iter"	{ return ITER; }
"translation"	{ return TRANSLATION; }
"rotation"	{ return ROTATION; }

.	{ printf("unknown token %c\n", yytext[0]); }