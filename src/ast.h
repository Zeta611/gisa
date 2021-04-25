#ifndef AST_H
#define AST_H

typedef struct ASTNode {
	enum { INIT_T,
	       TRANSLATION_T,
	       ROTATION_T,
	       SEQUENCE_T,
	       OR_T,
	       ITER_T,
	       REGION_T,
	       INTERVAL_T,
	       NUM_T } type;
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
		// NUM_T
		long long num;
	} u;
} ASTNode;

// Initialize `INIT_T` ASTNode. Returns `NULL` if failed.
ASTNode *init_node(ASTNode *region);

// Initialize `TRANSLATION_T` ASTNode. Returns `NULL` if failed.
ASTNode *translation_node(ASTNode *u, ASTNode *v);

// Initialize `ROTATION_T` ASTNode. Returns `NULL` if failed.
ASTNode *rotation_node(ASTNode *u, ASTNode *v, ASTNode *theta);

// Initialize `SEQUENCE_T` ASTNode. Returns `NULL` if failed.
ASTNode *sequence_node(ASTNode *p1, ASTNode *p2);

// Initialize `OR_T` ASTNode. Returns `NULL` if failed.
ASTNode *or_node(ASTNode *p1, ASTNode *p2);

// Initialize `ITER_T` ASTNode. Returns `NULL` if failed.
ASTNode *iter_node(ASTNode *body);

// Initialize `REGION_T` ASTNode. Returns `NULL` if failed.
ASTNode *region_node(ASTNode *t1, ASTNode *t2);

// Initialize `INTERVAL_T` ASTNode. Returns `NULL` if failed.
ASTNode *interval_node(ASTNode *n1, ASTNode *n2);

// Initialize `NUM_T` ASTNode. Returns `NULL` if failed.
ASTNode *num_node(long long num);

// Print an AST.
// You can use the output and pipe it into a LISP, e.g., Chicken Scheme. For
// example, to pretty print, you can use the following command:
// `echo "(import (chicken pretty-print)) (pp '$(progname input))" | csi`
// where `progname` may be `./build/parser` depending on how you invoke the
// program.
void print_ast(const ASTNode *ast);

#endif /* ifndef AST_H */