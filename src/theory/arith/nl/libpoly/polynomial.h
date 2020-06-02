
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__POLYNOMIAL_H
#define CVC4__THEORY__NLARITH__LIBPOLY__POLYNOMIAL_H

#include <poly/monomial.h>
#include <poly/polynomial.h>

#include <iostream>
#include <vector>

#include "integer.h"
#include "utils.h"
#include "variable.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/** A deleter for an std::unique_ptr holding a lp_polynomial_t pointer */
inline void polynomial_deleter(lp_polynomial_t* ptr)
{
  lp_polynomial_delete(ptr);
}

/**
 * Implements a wrapper for lp_polynomial_t from libpoly.
 */
class Polynomial
{
  friend std::ostream& operator<<(std::ostream& os, const Polynomial& p);
  /** The actual polynomial. */
  deleting_unique_ptr<lp_polynomial_t> mPoly;

 public:
  /** Construct a zero polynomial. */
  Polynomial()
      : mPoly(lp_polynomial_new(polynomial_ctx.get()), polynomial_deleter)
  {
  }
  /** Copy from the given Interval. */
  Polynomial(const Polynomial& p)
      : mPoly(lp_polynomial_new_copy(p.get()), polynomial_deleter)
  {
  }
  /** Create from a lp_polynomial_t pointer, claiming it's ownership. */
  Polynomial(lp_polynomial_t* poly) : mPoly(poly, polynomial_deleter) {}
  /** Create from a variable. */
  Polynomial(Variable v) : Polynomial(1, v, 1) {}
  /** Construct i * v^n. */
  Polynomial(Integer i, Variable v, unsigned n)
      : mPoly(lp_polynomial_alloc(), polynomial_deleter)
  {
    lp_polynomial_construct_simple(
        get(), polynomial_ctx.get(), i.get(), v.get(), n);
  }

  /** Assign from the given Polynomial. */
  Polynomial& operator=(const Polynomial& p)
  {
    mPoly.reset(lp_polynomial_new_copy(p.get()));
    return *this;
  }

  /** Get a non-const pointer to the internal lp_polynomial_t. Handle with care!
   */
  lp_polynomial_t* get() { return mPoly.get(); }
  /** Get a const pointer to the internal lp_polynomial_t. */
  const lp_polynomial_t* get() const { return mPoly.get(); }

  /**
   * Simplify the polynomial, assuming that we onle care about about its roots.
   * In particular, we divide by the gcd of the coefficients.
   */
  void simplify()
  {
    lp_polynomial_t* tmp = lp_polynomial_new(polynomial_ctx.get());
    lp_polynomial_pp(tmp, mPoly.get());
    mPoly.reset(tmp);
  }
};

/** Stream the given Polynomial to an output stream. */
inline std::ostream& operator<<(std::ostream& os, const Polynomial& p)
{
  return os << lp_polynomial_to_string(p.get());
}
/** Compare polynomials for equality. */
inline bool operator==(const Polynomial& lhs, const Polynomial& rhs)
{
  return lp_polynomial_cmp(lhs.get(), rhs.get()) == 0;
}
/** Compare polynomials. */
inline bool operator<(const Polynomial& lhs, const Polynomial& rhs)
{
  return lp_polynomial_cmp(lhs.get(), rhs.get()) < 0;
}

/** Add two polynomials. */
inline Polynomial operator+(const Polynomial& lhs, const Polynomial& rhs)
{
  Polynomial res;
  lp_polynomial_add(res.get(), lhs.get(), rhs.get());
  return res;
}
/** Add a polynomial and an integer. */
inline Polynomial operator+(const Polynomial& lhs, const Integer& rhs)
{
  lp_monomial_t monomial;
  lp_monomial_construct(polynomial_ctx.get(), &monomial);
  lp_monomial_set_coefficient(polynomial_ctx.get(), &monomial, rhs.get());
  Polynomial res(lhs);
  lp_polynomial_add_monomial(res.get(), &monomial);
  lp_monomial_destruct(&monomial);
  return res;
}
/** Add an integer and a polynomial. */
inline Polynomial operator+(const Integer& lhs, const Polynomial& rhs)
{
  return rhs + lhs;
}

/** Unary negation of a polynomial. */
inline Polynomial operator-(const Polynomial& p)
{
  Polynomial res;
  lp_polynomial_neg(res.get(), p.get());
  return res;
}
/** Subtract two polynomials. */
inline Polynomial operator-(const Polynomial& lhs, const Polynomial& rhs)
{
  Polynomial res;
  lp_polynomial_sub(res.get(), lhs.get(), rhs.get());
  return res;
}
/** Subtract an integer from a polynomial. */
inline Polynomial operator-(const Polynomial& lhs, const Integer& rhs)
{
  return lhs + (-rhs);
}
/** Subtract a polynomial from an integer. */
inline Polynomial operator-(const Integer& lhs, const Polynomial& rhs)
{
  return -rhs + lhs;
}

/** Multiply two polynomials. */
inline Polynomial operator*(const Polynomial& lhs, const Polynomial& rhs)
{
  Polynomial res;
  lp_polynomial_mul(res.get(), lhs.get(), rhs.get());
  return res;
}
/** Multiply a polynomial and an integer. */
inline Polynomial operator*(const Polynomial& lhs, const Integer& rhs)
{
  Polynomial res;
  lp_polynomial_mul_integer(res.get(), lhs.get(), rhs.get());
  return res;
}
/** Multiply an integer and a polynomial. */
inline Polynomial operator*(const Integer& lhs, const Polynomial& rhs)
{
  return rhs * lhs;
}

/** Divide a polynomial by a polynomial, assuming that there is no remainder. */
inline Polynomial div(const Polynomial& lhs, const Polynomial& rhs)
{
  Polynomial res;
  lp_polynomial_div(res.get(), lhs.get(), rhs.get());
  return res;
}

/** Check if the given polynomial is constant. */
inline bool is_constant(const Polynomial& p)
{
  return lp_polynomial_is_constant(p.get());
}
/** Obtain the degree of the given polynomial in its main variable. */
inline std::size_t degree(const Polynomial& p)
{
  return lp_polynomial_degree(p.get());
}
/** Obtain the k'th coefficient of a polynomial. */
inline Polynomial coefficient(const Polynomial& p, std::size_t k)
{
  Polynomial res;
  lp_polynomial_get_coefficient(res.get(), p.get(), k);
  return res;
}
/** Obtain the leading coefficient of a polynomial. */
inline Polynomial leading_coefficient(const Polynomial& p)
{
  return coefficient(p, degree(p));
}
/** Obtain all coefficients of a polynomial. */
inline std::vector<Polynomial> coefficients(const Polynomial& p)
{
  std::vector<Polynomial> res;
  for (std::size_t deg = 0; deg <= degree(p); ++deg)
  {
    auto coeff = coefficient(p, deg);
    if (lp_polynomial_is_constant(coeff.get())) continue;
    res.emplace_back(coeff);
  }
  return res;
}

/** Compute the derivative of a polynomial (in its main variable). */
inline Polynomial derivative(const Polynomial& p)
{
  Polynomial res;
  lp_polynomial_derivative(res.get(), p.get());
  return res;
}

/** Compute the resultant of two polynomials. */
inline Polynomial resultant(const Polynomial& p, const Polynomial& q)
{
  Polynomial res;
  lp_polynomial_resultant(res.get(), p.get(), q.get());
  return res;
}

/** Compute the discriminant of a polynomial. */
inline Polynomial discriminant(const Polynomial& p)
{
  return div(resultant(p, derivative(p)), leading_coefficient(p));
}

/**
 * Compute a square-free factorization of a polynomial.
 * Attention: this does not yield a full factorization!
 */
inline std::vector<Polynomial> square_free_factors(const Polynomial& p)
{
  lp_polynomial_t** factors = nullptr;
  std::size_t* multiplicities = nullptr;
  std::size_t size = 0;
  lp_polynomial_factor_square_free(p.get(), &factors, &multiplicities, &size);

  std::vector<Polynomial> res;
  for (std::size_t i = 0; i < size; ++i)
  {
    res.emplace_back(factors[i]);
  }
  free(factors);
  free(multiplicities);

  return res;
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
