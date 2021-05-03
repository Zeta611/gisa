#ifndef EVAL_H
#define EVAL_H
#include <stdbool.h>

typedef struct Env {
	bool init;
	double x;
	double y;
} Env;

struct ASTNode;
void eval(const struct ASTNode *ast, Env *env, int iter_max, bool verbose);

#endif /* ifndef EVAL_H */
