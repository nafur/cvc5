
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__UPOLYNOMIAL_H
#define CVC4__THEORY__NLARITH__LIBPOLY__UPOLYNOMIAL_H

#include <poly/integer.h>
#include <poly/upolynomial.h>

#include <iostream>
#include <vector>

#include "utils.h"
#include "variable.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/**
 * Implements a wrapper for lp_upolynomial_t from libpoly.
 */
class UPolynomial
{
  /** The actual univariate polynomial. */
  deleting_unique_ptr<lp_upolynomial_t> mPoly;

 public:
  /** Create from a lp_upolynomial_t pointer, claiming it's ownership. */
  UPolynomial(lp_upolynomial_t* poly);
  /** Create from a degree and a list of coefficients. */
  UPolynomial(std::size_t degree, const int* coeffs);
  /** Create from a degree and a list of coefficients. */
  UPolynomial(std::size_t degree, const lp_integer_t* coeffs);
  /** Copy from the given UPolynomial. */
  UPolynomial(const UPolynomial& poly);

  /** Get a non-const pointer to the internal lp_upolynomial_t. Handle with
   * care! */
  lp_upolynomial_t* get();
  /** Get a const pointer to the internal lp_upolynomial_t. */
  const lp_upolynomial_t* get() const;
  /** Release the lp_upolynomial_t pointer. This yields ownership of the
   * returned pointer. */
  lp_upolynomial_t* release();
};

/** Stream the given UPolynomial to an output stream. */
std::ostream& operator<<(std::ostream& os, const UPolynomial& p);

/** Add two univariate polynomials. */
UPolynomial operator+(const UPolynomial& lhs, const UPolynomial& rhs);
/** Subtract two univariate polynomials. */
UPolynomial operator-(const UPolynomial& lhs, const UPolynomial& rhs);
/** Multiply two univariate polynomials. */
UPolynomial operator*(const UPolynomial& lhs, const UPolynomial& rhs);

/** Obtain the degree of the given univariate polynomial. */
std::size_t degree(const UPolynomial& p);

/**
 * Compute a square-free factorization of a univariate polynomial.
 * Attention: this does not yield a full factorization!
 */
std::vector<UPolynomial> square_free_factors(const UPolynomial& p,
                                             bool with_constant = false);

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
