%{
#include "parser.tab.h"
#include <stdio.h>
#include <ctype.h>

extern char *yytext;
extern int lineno;
%}

ws	[ \t]+

digit	[0-9]
int	{digit}+
pow	[eE][+-]?{int}
real	(({int}\.?)|({digit}*\.{int})){pow}?

op	[-+*/^]
par	[\[\](){}]
%%

{ws}	{ ; }	// skip whitespaces
\n	{ ++lineno; }

{real}	{ sscanf(yytext, "%lf", &yylval.num); return NUM; }

{op}|{par}|,|;	{ return yytext[0]; }

[xyXY]	{ yylval.var = toupper(yytext[0]); return VAR; }

"or"	{ return OR; }
"init"	{ return INIT; }
"iter"	{ return ITER; }
"translation"	{ return TRANSLATION; }
"rotation"	{ return ROTATION; }

.	{ fprintf(stderr, "unknown token %c\n", yytext[0]); return ERR; }
