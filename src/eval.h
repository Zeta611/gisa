#ifndef EVAL_H
#define EVAL_H

typedef struct Env {
	double x;
	double y;
} Env;

struct ASTNode;
void eval(const struct ASTNode *ast, Env *env);

#endif /* ifndef EVAL_H */
