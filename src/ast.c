#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h>
#define EPSILON 1E-6

// Initialize `INIT_T` ASTNode. Returns `NULL` if failed.
ASTNode *init_node(ASTNode **nlist, ASTNode *region)
{
	ASTNode *n = malloc(sizeof *n);
	if (!n) {
		return NULL;
	}
	*n = (ASTNode){INIT_T, .u.init_region = region,
		       .next = *nlist ? *nlist : NULL};
	*nlist = n;
	return n;
}

// Initialize `TRANSLATION_T` ASTNode. Returns `NULL` if failed.
ASTNode *translation_node(ASTNode **nlist, ASTNode *u, ASTNode *v)
{
	ASTNode *n = malloc(sizeof *n);
	if (!n) {
		return NULL;
	}
	*n = (ASTNode){TRANSLATION_T, .u.translation_args = {u, v},
		       .next = *nlist ? *nlist : NULL};
	*nlist = n;
	return n;
}

// Initialize `ROTATION_T` ASTNode. Returns `NULL` if failed.
ASTNode *rotation_node(ASTNode **nlist, ASTNode *u, ASTNode *v, ASTNode *theta)
{
	ASTNode *n = malloc(sizeof *n);
	if (!n) {
		return NULL;
	}
	*n = (ASTNode){ROTATION_T, .u.rotation_args = {u, v, theta},
		       .next = *nlist ? *nlist : NULL};
	*nlist = n;
	return n;
}

// Initialize `SEQUENCE_T` ASTNode. Returns `NULL` if failed.
ASTNode *sequence_node(ASTNode **nlist, ASTNode *p1, ASTNode *p2)
{
	ASTNode *n = malloc(sizeof *n);
	if (!n) {
		return NULL;
	}
	*n = (ASTNode){SEQUENCE_T, .u.sequence_ps = {p1, p2},
		       .next = *nlist ? *nlist : NULL};
	*nlist = n;
	return n;
}

// Initialize `OR_T` ASTNode. Returns `NULL` if failed.
ASTNode *or_node(ASTNode **nlist, ASTNode *p1, ASTNode *p2)
{
	ASTNode *n = malloc(sizeof *n);
	if (!n) {
		return NULL;
	}
	*n = (ASTNode){OR_T, .u.or_ps = {p1, p2},
		       .next = *nlist ? *nlist : NULL};
	*nlist = n;
	return n;
}

// Initialize `ITER_T` ASTNode. Returns `NULL` if failed.
ASTNode *iter_node(ASTNode **nlist, ASTNode *body)
{
	ASTNode *n = malloc(sizeof *n);
	if (!n) {
		return NULL;
	}
	*n = (ASTNode){ITER_T, .u.iter_body = body,
		       .next = *nlist ? *nlist : NULL};
	*nlist = n;
	return n;
}

// Initialize `REGION_T` ASTNode. Returns `NULL` if failed.
ASTNode *region_node(ASTNode **nlist, ASTNode *t1, ASTNode *t2)
{
	ASTNode *n = malloc(sizeof *n);
	if (!n) {
		return NULL;
	}
	*n = (ASTNode){REGION_T, .u.region_ts = {t1, t2},
		       .next = *nlist ? *nlist : NULL};
	*nlist = n;
	return n;
}

// Initialize `INTERVAL_T` ASTNode. Returns `NULL` if failed.
ASTNode *interval_node(ASTNode **nlist, ASTNode *n1, ASTNode *n2)
{
	ASTNode *n = malloc(sizeof *n);
	if (!n) {
		return NULL;
	}
	*n = (ASTNode){INTERVAL_T, .u.interval_ns = {n1, n2},
		       .next = *nlist ? *nlist : NULL};
	*nlist = n;
	return n;
}

// Initialize `NUM_T` ASTNode. Returns `NULL` if failed.
ASTNode *num_node(ASTNode **nlist, double num)
{
	ASTNode *n = malloc(sizeof *n);
	if (!n) {
		return NULL;
	}
	*n = (ASTNode){NUM_T, .u.num = num, .next = *nlist ? *nlist : NULL};
	*nlist = n;
	return n;
}

// Print the S-expression of the AST `ast`.
void p_sexp_ast(const ASTNode *ast)
{
	putchar('(');
	switch (ast->type) {
	case INIT_T:
		fputs("init ", stdout);
		p_sexp_ast(ast->u.init_region);
		break;
	case TRANSLATION_T:
		fputs("translation ", stdout);
		p_sexp_ast(ast->u.translation_args.u);
		putchar(' ');
		p_sexp_ast(ast->u.translation_args.v);
		break;
	case ROTATION_T:
		fputs("rotation ", stdout);
		p_sexp_ast(ast->u.rotation_args.u);
		putchar(' ');
		p_sexp_ast(ast->u.rotation_args.v);
		putchar(' ');
		p_sexp_ast(ast->u.rotation_args.theta);
		break;
	case SEQUENCE_T:
		fputs("sequence ", stdout);
		p_sexp_ast(ast->u.sequence_ps.p1);
		putchar(' ');
		p_sexp_ast(ast->u.sequence_ps.p2);
		break;
	case OR_T:
		fputs("or ", stdout);
		p_sexp_ast(ast->u.or_ps.p1);
		putchar(' ');
		p_sexp_ast(ast->u.or_ps.p2);
		break;
	case ITER_T:
		fputs("iter ", stdout);
		p_sexp_ast(ast->u.iter_body);
		break;
	case REGION_T:
		fputs("region ", stdout);
		p_sexp_ast(ast->u.region_ts.t1);
		putchar(' ');
		p_sexp_ast(ast->u.region_ts.t2);
		break;
	case INTERVAL_T:
		fputs("interval ", stdout);
		p_sexp_ast(ast->u.interval_ns.n1);
		putchar(' ');
		p_sexp_ast(ast->u.interval_ns.n2);
		break;
	case NUM_T:
		fputs("num ", stdout);
		if (fmod(ast->u.num, 1.) < EPSILON) {
			printf("%.0lf", ast->u.num);
		} else if (fabs(ast->u.num) < 10000.) {
			printf("%lf", ast->u.num);
		} else {
			printf("%e", ast->u.num);
		}
		break;
	}
	putchar(')');
}

void free_nodes(ASTNode *nlist)
{
	if (!nlist) {
		return;
	}
	free_nodes(nlist->next);
	free(nlist);
}
