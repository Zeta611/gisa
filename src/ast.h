#ifndef AST_H
#define AST_H
#include <stdio.h>

typedef enum Var { VX = 'X', VY = 'Y' } Var;

typedef struct ASTNode {
	enum { INIT_T,
	       TRANSLATION_T,
	       ROTATION_T,
	       SEQUENCE_T,
	       OR_T,
	       ITER_T,
	       REGION_T,
	       INTERVAL_T,
	       OP_T,
	       NUM_T,
	       VAR_T } type;
	union {
		// INIT_T
		struct ASTNode *init_region;
		// TRANSLATION_T
		struct {
			struct ASTNode *u;
			struct ASTNode *v;
		} translation_args;
		// ROTATION_T
		struct {
			struct ASTNode *u;
			struct ASTNode *v;
			struct ASTNode *theta;
		} rotation_args;
		// SEQUENCE_T
		struct {
			struct ASTNode *p1;
			struct ASTNode *p2;
		} sequence_ps;
		// OR_T
		struct {
			struct ASTNode *p1;
			struct ASTNode *p2;
		} or_ps;
		// ITER_T
		struct ASTNode *iter_body;
		// REGION_T,
		struct {
			struct ASTNode *t1;
			struct ASTNode *t2;
		} region_ts;
		// INTERVAL_T
		struct {
			struct ASTNode *n1;
			struct ASTNode *n2;
		} interval_ns;
		// OP_T
		struct {
			enum Op { ADD, SUB, MUL, DIV, POW, NEG } op;
			struct ASTNode *larg, *rarg;
		} op_dat;
		// NUM_T
		double num;
		// VAR_T
		Var var;
	} u;
	struct ASTNode *next;
} ASTNode;

// Initialize `INIT_T` ASTNode. Returns `NULL` if failed.
ASTNode *init_node(ASTNode **nlist, ASTNode *region);

// Initialize `TRANSLATION_T` ASTNode. Returns `NULL` if failed.
ASTNode *translation_node(ASTNode **nlist, ASTNode *u, ASTNode *v);

// Initialize `ROTATION_T` ASTNode. Returns `NULL` if failed.
ASTNode *rotation_node(ASTNode **nlist, ASTNode *u, ASTNode *v, ASTNode *theta);

// Initialize `SEQUENCE_T` ASTNode. Returns `NULL` if failed.
ASTNode *sequence_node(ASTNode **nlist, ASTNode *p1, ASTNode *p2);

// Initialize `OR_T` ASTNode. Returns `NULL` if failed.
ASTNode *or_node(ASTNode **nlist, ASTNode *p1, ASTNode *p2);

// Initialize `ITER_T` ASTNode. Returns `NULL` if failed.
ASTNode *iter_node(ASTNode **nlist, ASTNode *body);

// Initialize `REGION_T` ASTNode. Returns `NULL` if failed.
ASTNode *region_node(ASTNode **nlist, ASTNode *t1, ASTNode *t2);

// Initialize `INTERVAL_T` ASTNode. Returns `NULL` if failed.
ASTNode *interval_node(ASTNode **nlist, ASTNode *n1, ASTNode *n2);

// Initialize `OP_T` ASTNode. Returns `NULL` if failed.
ASTNode *op_node(ASTNode **nlist, enum Op op, ASTNode *larg, ASTNode *rarg);

// Initialize `INUM_T` ASTNode. Returns `NULL` if failed.
ASTNode *inum_node(ASTNode **nlist, long inum);

// Initialize `NUM_T` ASTNode. Returns `NULL` if failed.
ASTNode *num_node(ASTNode **nlist, double num);

// Initialize `VAR_T` ASTNode. Returns `NULL` if failed.
ASTNode *var_node(ASTNode **nlist, Var var);

// Print the S-expression of the AST `ast` to `stream`.
// You can use the output and pipe it into a LISP, e.g., Chicken Scheme. For
// example, to pretty print, you can use the following command:
// `echo "(import (chicken pretty-print)) (pp '$(progname input))" | csi`
// where `progname` may be `./build/parser` depending on how you invoke the
// program.
void p_sexp_ast(FILE *stream, const ASTNode *ast);

void free_nodes(ASTNode *nlist);

#endif /* ifndef AST_H */
