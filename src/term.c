#include "term.h"
#include "util.h"
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

// All numbers less than or equal to `DBL_PRECISE_MAX` are precisely
// representable with `double`.
#define DBL_PRECISE_MAX ((double)(1L << DBL_MANT_DIG))

// Forward declarations for static functions
static int var_cmp(const TermNode *t1, const TermNode *t2);
static void add_coeff(TermNode *dest, const TermNode *src);
static void mul_coeff(TermNode *dest, const TermNode *src);
static void div_coeff(TermNode *dest, const TermNode *src);

static bool zero(TermNode *t);
static bool reduce0(TermNode **p);

static TermNode *term_dup(const TermNode *t);
static TermNode *var_dup(const TermNode *v);

static void mul_var(TermNode **dest, TermNode *src);

static void pow_num(TermNode **dest, TermNode *src);
static bool ipow_poly(TermNode **dest, long long exp);

static void free_term(TermNode *t);
static void print_var(const TermNode *v);

TermNode *coeff_term(double val)
{
	TermNode *term = malloc(sizeof *term);
	if (!term) {
		return NULL;
	}
	*term = (TermNode){COEFF_TERM, .hd.val = val, .u.vars = NULL, NULL};
	return term;
}

TermNode *var_term(char name, long pow)
{
	TermNode *term = malloc(sizeof *term);
	if (!term) {
		return NULL;
	}
	*term = (TermNode){VAR_TERM, .hd.name = name, .u.pow = pow, NULL};
	return term;
}

bool num_poly(const TermNode *p) { return p && !p->next && !p->u.vars; }

// First prioritize reverse-lexicographically, then prioritize higher orders.
// `t1` and `t2` must be `VARM_TERM`s or `NULL`s.
static int var_cmp(const TermNode *t1, const TermNode *t2)
{
	if (!t1 && !t2) {
		return 0;
	} else if (t1 && !t2) {
		return 1;
	} else if (!t1 && t2) {
		return -1;
	}
	// `t1->type` and `t2->type` must both be `VAR_TERM`s.
	int cmp = t1->hd.name - t2->hd.name;
	if (cmp) {
		// Prioritize reverse-lexicographically, e.g., 'x' > 'y' > 'z'.
		return -cmp;
	}
	// `t1` and `t2` start with a same variable term.
	cmp = (t1->u.pow > t2->u.pow) - (t1->u.pow < t2->u.pow);
	if (cmp) {
		return cmp;
	}
	return var_cmp(t1->next, t2->next);
}

int coeff_cmp(const TermNode *p1, const TermNode *p2)
{
	return ncmp(p1->hd.val, p2->hd.val);
}

int poly_cmp(const TermNode *p1, const TermNode *p2)
{
	if (!p1 && !p2) {
		return 0;
	} else if (p1 && !p2) {
		return 1;
	} else if (!p1 && p2) {
		return -1;
	}
	int cmp = var_cmp(p1->u.vars, p2->u.vars);
	if (cmp) {
		return cmp;
	}
	// `p1` and `p2` have the same highest-order term ignoring coefficients.
	cmp = coeff_cmp(p1, p2);
	if (cmp) {
		return cmp;
	}
	return poly_cmp(p1->next, p2->next);
}

static void add_coeff(TermNode *dest, const TermNode *src)
{
	dest->hd.val += src->hd.val;
}

static void mul_coeff(TermNode *dest, const TermNode *src)
{
	dest->hd.val *= src->hd.val;
}

// `src` should be guaranteed to contain a non-zero number.
static void div_coeff(TermNode *dest, const TermNode *src)
{
	dest->hd.val /= src->hd.val;
}

static bool zero(TermNode *t) { return t->hd.val == 0.; }

// Remove zero-terms from `*p`. If `*p` is equivalent to 0, it reduces to a
// single coefficient term of value 0.
// This should rarely fail in practice, since it allocates memory only after
// it has already freed more memory.
static bool reduce0(TermNode **p)
{
	TermNode **hd = p;
	while (*p) {
		if (zero(*p)) {
			TermNode *del = *p;
			*p = del->next;
			free_term(del);
		} else {
			p = &(*p)->next;
		}
	}
	if (!*hd) {
		// `*p` was equivalent to 0, and every term has been removed.
		*hd = coeff_term(0.);
		if (!*hd) { // `malloc` failed.
			return false;
		}
	}
	return true;
}

// Add `src` to `dest`.
// This is essentially merging two linked lists.
// Argument passed to `src` must not be used after `add_poly` is called.
// `TermNode`s composing the polynomial represented by `src` are either rewired
// to `dest` accordingly or completely released from memory.
bool add_poly(TermNode **dest, TermNode *src)
{
	// `*p` is the head pointer initially; `next` of a `TermNode`, if
	// traversed.
	// The double-pointer allows a uniform handling of both the head
	// pointer and the `next` pointer, by storing an address of a pointer.
	TermNode **p = dest;
	while (src) {
		if (!*p) {
			// Link rest of `src` at the end.
			*p = src;
			break;
		}
		int cmp = var_cmp((*p)->u.vars, src->u.vars);
		if (cmp > 0) {
			p = &(*p)->next;
		} else if (cmp < 0) {
			// Rewire `*src` to the `**p`-side.
			TermNode *tmp = src->next;
			src->next = *p;
			*p = src;
			src = tmp;
		} else {
			// Increase the coefficient of `**p` and release `src`.
			add_coeff(*p, src);
			p = &(*p)->next;
			TermNode *tmp = src;
			src = src->next;
			free_term(tmp);
		}
	}
	return reduce0(dest);
}

// Subtract `src` to `dest`.
// Argument passed to `src` must not be used after `sub_poly` is called.
bool sub_poly(TermNode **dest, TermNode *src)
{
	return neg_poly(src) && add_poly(dest, src);
}

static TermNode *term_dup(const TermNode *t)
{
	switch (t->type) {
	case COEFF_TERM:
		return coeff_term(t->hd.val);
	case VAR_TERM:
		return var_term(t->hd.name, t->u.pow);
	default:
		assert(false && "Unexpected node type\n");
	}
}

static TermNode *var_dup(const TermNode *v)
{
	TermNode *hd, **dup;
	dup = NULL;
	for (hd = term_dup(v); v; v = v->next, dup = &(*dup)->next) {
		if (!dup) {
			dup = &hd;
		} else {
			*dup = term_dup(v);
		}
		if (!*dup) { // `term_dup` failed either for `hd` or `v`.
			free_term(hd);
			return NULL;
		}
	}
	return hd;
}

// Duplicate `p`.
TermNode *poly_dup(const TermNode *p)
{
	TermNode *hd, **dup;
	dup = NULL;
	for (hd = term_dup(p); p; p = p->next, dup = &(*dup)->next) {
		if (!dup) {
			dup = &hd;
		} else {
			*dup = term_dup(p);
		}
		if (!*dup) { // `term_dup` failed either for `hd` or `p`.
			goto dup_fail;
		}
		TermNode **vdup = &(*dup)->u.vars;
		for (TermNode *vt = p->u.vars; vt; vt = vt->next) {
			*vdup = term_dup(vt);
			if (!*vdup) { // `term_dup` failed.
				goto dup_fail;
			}
			vdup = &(*vdup)->next;
		}
	}
	return hd;
dup_fail:
	free_poly(hd);
	return NULL;
}

// Multiply `src` to `dest`--both should point directly to `VAR_TERM`s.
// This is analagous to `add_poly` function, as it is merging two sorted
// `VAR_TERM` lists.
static void mul_var(TermNode **dest, TermNode *src)
{
	TermNode **p = dest;
	while (src) {
		if (!*p) {
			*p = src;
			return;
		}
		// Again, prioritize reverse-lexicographically.
		int cmp = -((*p)->hd.name - src->hd.name);
		if (cmp > 0) {
			p = &(*p)->next;
		} else if (cmp < 0) {
			TermNode *tmp = src->next;
			src->next = *p;
			*p = src;
			src = tmp;
		} else {
			(*p)->u.pow += src->u.pow;
			p = &(*p)->next;
			TermNode *tmp = src;
			src = src->next;
			// No dedicated function to free a single `VAR_TERM`.
			free(tmp);
		}
	}
}

// Multiply `src` to `dest`.
// Uses distributive law to multiply.
// Argument passed to `src` must not be used after `mul_poly` is called.
// On failure, it frees all memory held by `src` to prevent malformed list
// structure. Although `*dest` is properly formed, it does not hold a correct
// value, so the caller needs to free it manually upon failure.
bool mul_poly(TermNode **dest, TermNode *src)
{
	// `dup` points to the head pointer initially, and then points to the
	// copy of `*dest` afterwords (only if there are more than one term
	// in `src` to apply distributive law).
	// `dup` is there only to keep a pointer later to be assigned to `p`.
	TermNode **dup, **p;
	for (dup = p = dest; src; p = dup) {
		if (src->next) {
			TermNode *tmp = poly_dup(*dup);
			if (!tmp) {
				goto dup_fail;
			}
			// Get a fresh slot for a head pointer container.
			dup = malloc(sizeof *dup);
			if (!dup) {
				free_poly(tmp);
				goto dup_fail;
			}
			*dup = tmp;
		}
		TermNode *svars = src->u.vars;
		// Multiply a term `*src` to each of the terms in `*p`.
		for (TermNode **i = p; *i; i = &(*i)->next) {
			mul_coeff(*i, src);
			if (svars) {
				TermNode *vdup = var_dup(svars);
				if (!vdup) {
					if (p != dest) {
						free_poly(*p);
						free(p);
					}
					goto dup_fail;
				}
				mul_var(&(*i)->u.vars, vdup);
			}
		}
		if (p != dest) {
			bool success = add_poly(dest, *p);
			free(p); // Allocated by `dup`.
			if (!success) {
				goto dup_fail;
			}
		}
		TermNode *tmp = src;
		src = src->next;
		free_term(tmp);
	}
	return reduce0(dest);
dup_fail:
	free_poly(src);
	return false;
}

// Divide `src` to `dest`.
// Argument passed to `src` must not be used after `div_poly` is called.
bool div_poly(TermNode **dest, TermNode *src)
{
	bool success = true;
	if (src->u.vars) {
		fputs("Division by a polynomial is not supported.\n", stderr);
		success = false;
		goto src_cleanup;
	}
	if (zero(src)) {
		fputs("Division by ZERO.\n", stderr);
		success = false;
		goto src_cleanup;
	}
	for (; *dest; dest = &(*dest)->next) {
		div_coeff(*dest, src);
	}
src_cleanup:
	free_poly(src);
	return success;
}

// Both `*dest` and `src` should be number terms.
static void pow_num(TermNode **dest, TermNode *src)
{
	(*dest)->hd.val = pow((*dest)->hd.val, src->hd.val);
}

// `exp` should be a positive integer.
static bool ipow_poly(TermNode **dest, long long exp)
{
	if (exp < 2) {
		return true;
	}
	bool success = true;
	TermNode *dup = NULL;
	if (exp % 2) {
		dup = poly_dup(*dest);
		if (!dup) {
			return false;
		}
		success = ipow_poly(dest, exp - 1);
	} else {
		success = ipow_poly(dest, exp / 2);
		if (success) {
			dup = poly_dup(*dest);
			if (!dup) {
				return false;
			}
		}
	}
	return success && mul_poly(dest, dup);
}

// Exponentiate `src` to `dest`.
// Argument passed to `src` must not be used after `pow_poly` is called.
bool pow_poly(TermNode **dest, TermNode *src)
{
	bool success = true;
	if (src->u.vars) {
		fputs("Exponentiation with a polynomial is not supported.\n",
		      stderr);
		success = false;
		goto src_cleanup;
	}
	if (num_poly(*dest)) {
		pow_num(dest, src);
		goto src_cleanup;
	}

	const double fexp = src->hd.val;
	if (fexp != floor(fexp)) {
		fputs("Exponentiation with a polynomial and a real number is "
		      "not supported.\n",
		      stderr);
		success = false;
		goto src_cleanup;
	}
	if (fexp < 0) {
		fputs("Exponentiation with a polynomial and a negative "
		      "integer is not supported.\n",
		      stderr);
		success = false;
		goto src_cleanup;
	}
	if (fexp >= DBL_PRECISE_MAX) {
		fputs("Exponent out of range.\n", stderr);
		success = false;
		goto src_cleanup;
	}

	const long long exp = fexp; // `long` is not enough on Windows machines.
	if (exp == 0) {
		TermNode *tmp = *dest;
		*dest = coeff_term(1.);
		if (!*dest) {
			success = false;
		}
		free_poly(tmp);
	} else {
		success = ipow_poly(dest, exp);
	}
src_cleanup:
	free_poly(src);
	return success;
}

// Negate `dest`.
bool neg_poly(TermNode *dest)
{
	for (; dest; dest = dest->next) {
		dest->hd.val = -dest->hd.val;
	}
	return true;
}

// Release a single `COEFF_TERM` and/or all linked `VAR_TERM`s.
// Does not recursively release linked `COEFF_TERM` terms. For that purpose, use
// `free_poly`.
static void free_term(TermNode *t)
{
	if (!t) {
		return;
	}
	switch (t->type) {
	case COEFF_TERM:
		free_term(t->u.vars);
		break;
	case VAR_TERM:
		free_term(t->next);
		break;
	default:
		assert(false && "Unexpected node type\n");
	}
	free(t);
}

static void print_var(const TermNode *v)
{
	for (; v; v = v->next) {
		int p = v->u.pow;
		if (p == 1) {
			fputc(v->hd.name, stderr);
			fputc(' ', stderr);
		} else {
			fprintf(stderr, "%c^%d ", v->hd.name, p);
		}
	}
}

// Print a polynomial pointed by `p`.
void print_poly(const TermNode *p)
{
	while (p) {
		fprintf(stderr, "%lf ", p->hd.val);
		print_var(p->u.vars);
		p = p->next;
		if (p) {
			fputs("+ ", stderr);
		}
	}
}

// Release a polynomial, i.e., `COEFF_TERM` typed `TermNode` linked together.
void free_poly(TermNode *p)
{
	if (!p) {
		return;
	}
	free_poly(p->next);
	free_term(p);
}
