#include "eval.h"
#include "ast.h"
#include "term.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define CHK_EVAL_POLY(poly, ast, label)                                        \
	do {                                                                   \
		poly = eval_poly(ast);                                         \
		if (!poly) {                                                   \
			poly_err_msg(ast);                                     \
			ret = 3;                                               \
			goto label;                                            \
		}                                                              \
		if (!num_poly(poly)) {                                         \
			non_num_msg(ast, poly);                                \
			ret = 2;                                               \
			goto label;                                            \
		}                                                              \
	} while (0);

static double randf(double s, double e);
static int randi(int n);

static TermNode *eval_poly(const ASTNode *ast)
{
	if (!ast) { // for NEG op
		return NULL;
	}
	switch (ast->type) {
	case OP_T: {
		enum Op op = ast->u.op_dat.op;
		TermNode *lt = eval_poly(ast->u.op_dat.larg);
		TermNode *rt = eval_poly(ast->u.op_dat.rarg);

		// Result of `eval_poly` being `NULL` indicates an invalid
		// syntax or an operation, except for the result of evaluating
		// the right child of the `NEG` op.
		if (!lt || (!rt && op != NEG)) {
			free_poly(lt);
			free_poly(rt);
			return NULL;
		}

		bool success;
		switch (op) {
		case ADD:
			success = add_poly(&lt, rt);
			break;
		case SUB:
			success = sub_poly(&lt, rt);
			break;
		case MUL:
			success = mul_poly(&lt, rt);
			break;
		case DIV:
			success = div_poly(&lt, rt);
			break;
		case POW:
			success = pow_poly(&lt, rt);
			break;
		case NEG:
			success = neg_poly(lt);
			break;
		default:
			assert(false && "Unknown op type");
		}
		if (!success) {
			free_poly(lt);
			return NULL;
		}
		return lt;
	}
	case NUM_T: {
		TermNode *t = coeff_term(ast->u.num);
		if (!t) {
			goto mem_err;
		}
		return t;
	}
	case VAR_T: {
		TermNode *p = coeff_term(1.);
		if (!p) {
			goto mem_err;
		}
		TermNode *vt = var_term(ast->u.var, 1);
		if (!vt) {
			goto mem_err;
		}
		p->u.vars = vt;
		return p;
	}
	default:
		assert(false && "Unexpected node type");
	}
mem_err:
	fputs("Failed to allocate memory.\n", stderr);
	return NULL;
}

static void poly_err_msg(const ASTNode *ast)
{
	fputs("Evaluation of '", stderr);
	p_sexp_ast(stderr, ast);
	fputs("' failed\n", stderr);
}

static void non_num_msg(const ASTNode *ast, const TermNode *poly)
{
	fputs("Evaluation of '", stderr);
	p_sexp_ast(stderr, ast);
	fputs("' results in a non-number '", stderr);
	print_poly(poly);
	fputs("'\n", stderr);
}

int eval(const ASTNode *ast, Env *env, int iter_max, bool verbose)
{
	// 0: OK, 1: Uninitialized, 2: Non-number argument, 3: Polynomial error
	int ret = 0;
	switch (ast->type) {
	case INIT_T:
		env->init = true;
		ret = eval(ast->u.init_region, env, iter_max, verbose);
		break;
	case TRANSLATION_T: {
		if (!env->init) {
			return 1;
		}

		TermNode *u_poly = NULL;
		TermNode *v_poly = NULL;
		CHK_EVAL_POLY(u_poly, ast->u.translation_args.u, trans_cleanup);
		CHK_EVAL_POLY(v_poly, ast->u.translation_args.v, trans_cleanup);

		const double u = u_poly->hd.val;
		const double v = v_poly->hd.val;
		env->x += u;
		env->y += v;

		if (verbose) {
			fprintf(stderr, "Translate +(%lf, %lf) -> (%lf, %lf)\n",
				u, v, env->x, env->y);
		}
	trans_cleanup:
		free_poly(u_poly);
		free_poly(v_poly);
		break;
	}
	case ROTATION_T: {
		if (!env->init) {
			return 1;
		}
		TermNode *u_poly = NULL;
		TermNode *v_poly = NULL;
		TermNode *theta_poly = NULL;
		CHK_EVAL_POLY(u_poly, ast->u.rotation_args.u, rot_cleanup);
		CHK_EVAL_POLY(v_poly, ast->u.rotation_args.v, rot_cleanup);
		CHK_EVAL_POLY(theta_poly, ast->u.rotation_args.theta,
			      rot_cleanup);

		const double u = u_poly->hd.val;
		const double v = v_poly->hd.val;
		const double theta = theta_poly->hd.val;

		const double deg = theta / 180. * M_PI;
		const double s = sin(deg);
		const double c = cos(deg);

		// Subtract, rotate, and then add again.
		env->x -= u;
		env->y -= v;

		const double rx = env->x * c - env->y * s;
		const double ry = env->x * s + env->y * c;
		env->x = rx + u;
		env->y = ry + v;

		if (verbose) {
			fprintf(stderr,
				"Rotate @(%lf, %lf, %lfdeg) -> (%lf, %lf)\n", u,
				v, deg, env->x, env->y);
		}
	rot_cleanup:
		free_poly(u_poly);
		free_poly(v_poly);
		free_poly(theta_poly);
		break;
	}
	case SEQUENCE_T:
		ret = eval(ast->u.sequence_ps.p1, env, iter_max, verbose);
		if (ret) {
			return ret;
		}
		ret = eval(ast->u.sequence_ps.p2, env, iter_max, verbose);
		break;
	case OR_T: {
		if (!env->init) {
			return 1;
		}
		int right = randi(2);
		if (verbose) {
			fprintf(stderr, "OR selected %s\n",
				right ? "right" : "left");
		}
		if (right) {
			ret = eval(ast->u.or_ps.p2, env, iter_max, verbose);
		} else {
			ret = eval(ast->u.or_ps.p1, env, iter_max, verbose);
		}
		break;
	}
	case ITER_T: {
		if (!env->init) {
			return 1;
		}
		int iter = randi(iter_max + 1);
		if (verbose) {
			fprintf(stderr, "Iterate %d times\n", iter);
		}
		for (int i = 0; i < iter; ++i) {
			ret = eval(ast->u.iter_body, env, iter_max, verbose);
			if (ret) {
				return ret;
			}
		}
		break;
	}
	case REGION_T: {
		TermNode *xs_poly = NULL;
		TermNode *xe_poly = NULL;
		TermNode *ys_poly = NULL;
		TermNode *ye_poly = NULL;
		CHK_EVAL_POLY(xs_poly, ast->u.region_ts.t1->u.interval_ns.n1,
			      region_cleanup);
		CHK_EVAL_POLY(xe_poly, ast->u.region_ts.t1->u.interval_ns.n2,
			      region_cleanup);
		CHK_EVAL_POLY(ys_poly, ast->u.region_ts.t2->u.interval_ns.n1,
			      region_cleanup);
		CHK_EVAL_POLY(ye_poly, ast->u.region_ts.t2->u.interval_ns.n2,
			      region_cleanup);

		const double xs = xs_poly->hd.val;
		const double xe = xe_poly->hd.val;
		const double ys = ys_poly->hd.val;
		const double ye = ye_poly->hd.val;

		const double xr = randf(xs, xe);
		const double yr = randf(ys, ye);
		env->x = xr;
		env->y = yr;

		if (verbose) {
			fprintf(stderr,
				"Initialize in region [%lf, %lf] x [%lf, %lf]: "
				"(%lf, %lf)\n",
				xs, xe, ys, ye, xr, yr);
		}
	region_cleanup:
		free_poly(xs_poly);
		free_poly(xe_poly);
		free_poly(ys_poly);
		free_poly(ye_poly);
		break;
	}
	case INTERVAL_T:
		assert(false && "Invalid `ast->type`: `INTERVAL_T`");
		break;
	case OP_T:
		assert(false && "Invalid `ast->type`: `OP_T`");
		break;
	case NUM_T:
		assert(false && "Invalid `ast->type`: `NUM_T`");
		break;
	case VAR_T:
		assert(false && "Invalid `ast->type`: `VAR_T`");
		break;
	}
	return ret;
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
