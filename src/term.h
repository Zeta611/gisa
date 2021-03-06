#ifndef TERM_H
#define TERM_H

#include <stdbool.h>

/* Diagram of the representation for 2xy^2 + 5y + 9 using `TermNode`s
 *
 *  hd   u  next
 * +---+---+---+   +---+---+---+   +---+---+---+
 * | 2 | # | #-+-->| 5 | # | #-+-->| 9 | $ | $ |
 * +---+-|-+---+   +---+-|-+---+   +---+---+---+
 *       |               v
 *       |             +---+---+---+
 *       |             | y | 1 | $ |
 *       |             +---+---+---+
 *       v
 *     +---+---+---+   +---+---+---+
 *     | x | 1 | #-+-->| y | 2 | $ |
 *     +---+---+---+   +---+---+---+
 */
typedef struct TermNode {
	enum { COEFF_TERM, VAR_TERM } type;
	union {
		double val; // COEFF_TERM
		char name;  // VAR_TERM
	} hd;
	union {
		struct TermNode *vars; // COEFF_TERM
		long pow;	       // VAR_TERM
	} u;
	struct TermNode *next;
} TermNode;

TermNode *coeff_term(double val);

TermNode *var_term(char name, long pow);

bool num_poly(const TermNode *p);

int coeff_cmp(const TermNode *p1, const TermNode *p2);

// For each term, first prioritize reverse-lexicographically, and then
// prioritize higher orders. Compare the next term in case of a tie.
int poly_cmp(const TermNode *p1, const TermNode *p2);

// Duplicate `p`.
TermNode *poly_dup(const TermNode *p);

// Add `src` to `dest`.
// Argument passed to `src` must not be used after `add_poly` is called.
bool add_poly(TermNode **dest, TermNode *src);

// Subtract `src` to `dest`.
// Argument passed to `src` must not be used after `sub_poly` is called.
bool sub_poly(TermNode **dest, TermNode *src);

// Multiply `src` to `dest`.
// Argument passed to `src` must not be used after `mul_poly` is called.
bool mul_poly(TermNode **dest, TermNode *src);

// Divide `src` to `dest`.
// Argument passed to `src` must not be used after `div_poly` is called.
bool div_poly(TermNode **dest, TermNode *src);

// Exponentiate `src` to `dest`.
// Argument passed to `src` must not be used after `pow_poly` is called.
bool pow_poly(TermNode **dest, TermNode *src);

// Negate `dest`.
bool neg_poly(TermNode *dest);

// Print a polynomial pointed by `p`.
void print_poly(const TermNode *p);

// Release a polynomial, i.e., `COEFF_TERM` typed `TermNode` linked together.
void free_poly(TermNode *p);

#endif /* ifndef TERM_H */
