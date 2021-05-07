#ifndef EVAL_H
#define EVAL_H
#include <stdbool.h>

typedef struct Env {
	bool init;
	double x;
	double y;
} Env;

struct ASTNode;
// Returns 0 if successful; 1 if uninitialized, 2 if non-number argument for an
// argument expecting a number.
int eval(const struct ASTNode *ast, Env *env, int iter_max, bool verbose);

#endif /* ifndef EVAL_H */
