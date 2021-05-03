#include "eval.h"
#include "ast.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static double randf(double s, double e);
static int randi(int n);

void eval(const ASTNode *ast, Env *env, int iter_max, bool verbose)
{
	switch (ast->type) {
	case INIT_T:
		env->init = true;
		eval(ast->u.init_region, env, iter_max, verbose);
		break;
	case TRANSLATION_T: {
		if (!env->init) {
			return;
		}
		double u = ast->u.translation_args.u->u.num;
		double v = ast->u.translation_args.v->u.num;
		env->x += u;
		env->y += v;

		if (verbose) {
			fprintf(stderr, "Translate +(%lf, %lf) -> (%lf, %lf)\n",
				u, v, env->x, env->y);
		}
		break;
	}
	case ROTATION_T: {
		if (!env->init) {
			return;
		}
		double u = ast->u.rotation_args.u->u.num;
		double v = ast->u.rotation_args.v->u.num;
		double deg = ast->u.rotation_args.theta->u.num / 180. * M_PI;
		double s = sin(deg);
		double c = cos(deg);

		// Subtract, rotate, and then add again.
		env->x -= u;
		env->y -= v;

		double rx = env->x * c - env->y * s;
		double ry = env->x * s + env->y * c;

		env->x = rx + u;
		env->y = ry + v;
		if (verbose) {
			fprintf(stderr,
				"Rotate @(%lf, %lf, %lfdeg) -> (%lf, %lf)\n", u,
				v, deg, env->x, env->y);
		}
		break;
	}
	case SEQUENCE_T:
		eval(ast->u.sequence_ps.p1, env, iter_max, verbose);
		eval(ast->u.sequence_ps.p2, env, iter_max, verbose);
		break;
	case OR_T: {
		if (!env->init) {
			return;
		}
		int right = randi(2);
		if (verbose) {
			fprintf(stderr, "OR selected %s\n",
				right ? "right" : "left");
		}
		if (right) {
			eval(ast->u.or_ps.p2, env, iter_max, verbose);
		} else {
			eval(ast->u.or_ps.p1, env, iter_max, verbose);
		}
		break;
	}
	case ITER_T: {
		if (!env->init) {
			return;
		}
		int iter = randi(iter_max + 1);
		if (verbose) {
			fprintf(stderr, "Iterate %d times\n", iter);
		}
		for (int i = 0; i < iter; ++i) {
			eval(ast->u.iter_body, env, iter_max, verbose);
		}
		break;
	}
	case REGION_T: {
		double xs = ast->u.region_ts.t1->u.interval_ns.n1->u.num;
		double xe = ast->u.region_ts.t1->u.interval_ns.n2->u.num;
		double xr = randf(xs, xe);

		double ys = ast->u.region_ts.t2->u.interval_ns.n1->u.num;
		double ye = ast->u.region_ts.t2->u.interval_ns.n2->u.num;
		double yr = randf(ys, ye);

		env->x = xr;
		env->y = yr;

		if (verbose) {
			fprintf(stderr,
				"Initialize in region [%lf, %lf] x [%lf, %lf]: "
				"(%lf, %lf)\n",
				xs, xe, ys, ye, xr, yr);
		}
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

// Random integer from 0 to `n` - 1.
static int randi(int n)
{
	assert(n > 0);
	return (int)((double)rand() / ((double)RAND_MAX + 1) * n);
}
