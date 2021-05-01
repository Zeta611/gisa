#include "eval.h"
#include "ast.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h>

static double randf(double s, double e);

void eval(const ASTNode *ast, Env *env)
{
	switch (ast->type) {
	case INIT_T:
		eval(ast->u.init_region, env);
		break;
	case TRANSLATION_T: {
		double u = ast->u.translation_args.u->u.num;
		double v = ast->u.translation_args.v->u.num;
		env->x += u;
		env->y += v;
		break;
	}
	case ROTATION_T: {
		double u = ast->u.rotation_args.u->u.num;
		double v = ast->u.rotation_args.v->u.num;
		double theta = ast->u.rotation_args.theta->u.num;
		double s = sin(theta);
		double c = cos(theta);

		// Subtract, rotate, and then add again.
		env->x -= u;
		env->y -= v;

		double rx = env->x * c - env->y * s;
		double ry = env->x * s + env->y * c;

		env->x = rx + u;
		env->y = ry + v;
		break;
	}
	case SEQUENCE_T:
		eval(ast->u.sequence_ps.p1, env);
		eval(ast->u.sequence_ps.p2, env);
		break;
	case OR_T:
		if (rand() % 2) {
			eval(ast->u.or_ps.p1, env);
		} else {
			eval(ast->u.or_ps.p2, env);
		}
		break;
	case ITER_T:
		for (int i = 0; i < rand(); ++i) {
			eval(ast->u.iter_body, env);
		}
		break;
	case REGION_T: {
		double xs = ast->u.region_ts.t1->u.interval_ns.n1->u.num;
		double xe = ast->u.region_ts.t1->u.interval_ns.n2->u.num;
		double xr = randf(xs, xe);

		double ys = ast->u.region_ts.t2->u.interval_ns.n1->u.num;
		double ye = ast->u.region_ts.t2->u.interval_ns.n2->u.num;
		double yr = randf(ys, ye);

		env->x = xr;
		env->y = yr;
		break;
	}
	case INTERVAL_T:
		assert(false && "Invalid `ast->type`: `INTERVAL_T`");
		break;
	case NUM_T:
		assert(false && "Invalid `ast->type`: `NUM_T`");
		break;
	}
}

static double randf(double s, double e)
{
	return (e - s) * ((double)rand() / (double)RAND_MAX) + s;
}
