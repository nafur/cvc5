#include "polynomial.h"

#include <poly/feasibility_set.h>

#include "base/output.h"
#include "value.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/** A deleter for an std::unique_ptr holding a lp_polynomial_t pointer */
void polynomial_deleter(lp_polynomial_t* ptr) { lp_polynomial_delete(ptr); }

/** Construct a zero polynomial. */
Polynomial::Polynomial()
    : mPoly(lp_polynomial_new(polynomial_ctx.get()), polynomial_deleter)
{
}
/** Copy from the given Interval. */
Polynomial::Polynomial(const Polynomial& p)
    : mPoly(lp_polynomial_new_copy(p.get()), polynomial_deleter)
{
}
/** Create from a lp_polynomial_t pointer, claiming it's ownership. */
Polynomial::Polynomial(lp_polynomial_t* poly) : mPoly(poly, polynomial_deleter)
{
}
/** Create from a variable. */
Polynomial::Polynomial(Variable v) : Polynomial(1, v, 1) {}
/** Construct i * v^n. */
Polynomial::Polynomial(Integer i, Variable v, unsigned n)
    : mPoly(lp_polynomial_alloc(), polynomial_deleter)
{
  lp_polynomial_construct_simple(
      get(), polynomial_ctx.get(), i.get(), v.get(), n);
}
Polynomial::Polynomial(Integer i)
    : mPoly(lp_polynomial_alloc(), polynomial_deleter)
{
  lp_polynomial_construct_simple(
      get(), polynomial_ctx.get(), i.get(), lp_variable_null, 0);
}

/** Assign from the given Polynomial. */
Polynomial& Polynomial::operator=(const Polynomial& p)
{
  mPoly.reset(lp_polynomial_new_copy(p.get()));
  return *this;
}

/** Get a non-const pointer to the internal lp_polynomial_t. Handle with care!
 */
lp_polynomial_t* Polynomial::get() { return mPoly.get(); }
/** Get a const pointer to the internal lp_polynomial_t. */
const lp_polynomial_t* Polynomial::get() const { return mPoly.get(); }

/**
 * Simplify the polynomial, assuming that we onle care about about its roots.
 * In particular, we divide by the gcd of the coefficients.
 */
void Polynomial::simplify()
{
  lp_polynomial_t* tmp = lp_polynomial_new(polynomial_ctx.get());
  lp_polynomial_pp(tmp, mPoly.get());
  mPoly.reset(tmp);
}

/** Stream the given Polynomial to an output stream. */
std::ostream& operator<<(std::ostream& os, const Polynomial& p)
{
  return os << lp_polynomial_to_string(p.get());
}
/** Compare polynomials for equality. */
bool operator==(const Polynomial& lhs, const Polynomial& rhs)
{
  return lp_polynomial_cmp(lhs.get(), rhs.get()) == 0;
}
/** Compare polynomials. */
bool operator<(const Polynomial& lhs, const Polynomial& rhs)
{
  return lp_polynomial_cmp(lhs.get(), rhs.get()) < 0;
}

/** Add two polynomials. */
Polynomial operator+(const Polynomial& lhs, const Polynomial& rhs)
{
  Polynomial res;
  lp_polynomial_add(res.get(), lhs.get(), rhs.get());
  return res;
}
/** Add a polynomial and an integer. */
Polynomial operator+(const Polynomial& lhs, const Integer& rhs)
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
Polynomial operator+(const Integer& lhs, const Polynomial& rhs)
{
  return rhs + lhs;
}

/** Unary negation of a polynomial. */
Polynomial operator-(const Polynomial& p)
{
  Polynomial res;
  lp_polynomial_neg(res.get(), p.get());
  return res;
}
/** Subtract two polynomials. */
Polynomial operator-(const Polynomial& lhs, const Polynomial& rhs)
{
  Polynomial res;
  lp_polynomial_sub(res.get(), lhs.get(), rhs.get());
  return res;
}
/** Subtract an integer from a polynomial. */
Polynomial operator-(const Polynomial& lhs, const Integer& rhs)
{
  return lhs + (-rhs);
}
/** Subtract a polynomial from an integer. */
Polynomial operator-(const Integer& lhs, const Polynomial& rhs)
{
  return -rhs + lhs;
}

/** Multiply two polynomials. */
Polynomial operator*(const Polynomial& lhs, const Polynomial& rhs)
{
  Polynomial res;
  lp_polynomial_mul(res.get(), lhs.get(), rhs.get());
  return res;
}
/** Multiply a polynomial and an integer. */
Polynomial operator*(const Polynomial& lhs, const Integer& rhs)
{
  Polynomial res;
  lp_polynomial_mul_integer(res.get(), lhs.get(), rhs.get());
  return res;
}
/** Multiply an integer and a polynomial. */
Polynomial operator*(const Integer& lhs, const Polynomial& rhs)
{
  return rhs * lhs;
}

/** Multiply and assign two polynomials. */
Polynomial& operator*=(Polynomial& lhs, const Polynomial& rhs)
{
  lp_polynomial_mul(lhs.get(), lhs.get(), rhs.get());
  return lhs;
}

/** Compute a polynomial to some power. */
Polynomial pow(const Polynomial& lhs, unsigned exp)
{
  Polynomial res;
  lp_polynomial_pow(res.get(), lhs.get(), exp);
  return res;
}

/** Divide a polynomial by a polynomial, assuming that there is no remainder. */
Polynomial div(const Polynomial& lhs, const Polynomial& rhs)
{
  Polynomial res;
  lp_polynomial_div(res.get(), lhs.get(), rhs.get());
  return res;
}

/** Check if the given polynomial is constant. */
bool is_constant(const Polynomial& p)
{
  return lp_polynomial_is_constant(p.get());
}
/** Obtain the degree of the given polynomial in its main variable. */
std::size_t degree(const Polynomial& p)
{
  return lp_polynomial_degree(p.get());
}
/** Obtain the main variable of the given polynomial. */
Variable main_variable(const Polynomial& p)
{
  return lp_polynomial_top_variable(p.get());
}
/** Obtain the k'th coefficient of a polynomial. */
Polynomial coefficient(const Polynomial& p, std::size_t k)
{
  Polynomial res;
  lp_polynomial_get_coefficient(res.get(), p.get(), k);
  return res;
}
/** Obtain the leading coefficient of a polynomial. */
Polynomial leading_coefficient(const Polynomial& p)
{
  return coefficient(p, degree(p));
}
/** Obtain all non-constant coefficients of a polynomial. */
std::vector<Polynomial> coefficients(const Polynomial& p)
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
Polynomial derivative(const Polynomial& p)
{
  Polynomial res;
  lp_polynomial_derivative(res.get(), p.get());
  return res;
}

/** Compute the resultant of two polynomials. */
Polynomial resultant(const Polynomial& p, const Polynomial& q)
{
  Polynomial res;
  lp_polynomial_resultant(res.get(), p.get(), q.get());
  return res;
}

/** Compute the discriminant of a polynomial. */
Polynomial discriminant(const Polynomial& p)
{
  return div(resultant(p, derivative(p)), leading_coefficient(p));
}

std::vector<Polynomial> square_free_factors(const Polynomial& p)
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

bool evaluate_polynomial_constraint(const Polynomial& p,
                                    const Assignment& a,
                                    SignCondition sc)
{
  // Trace("cad-check") << "Get sign of " << p << " over " << a << std::endl;
  return evaluate_sign_condition(sc, lp_polynomial_sgn(p.get(), a.get()));
}

std::vector<Interval> infeasible_regions(const Polynomial& p,
                                         const Assignment& a,
                                         SignCondition sc)
{
  lp_feasibility_set_t* feasible = lp_polynomial_constraint_get_feasible_set(
      p.get(), to_sign_condition(sc), 0, a.get());

  std::vector<Interval> regions;

  Value last_value = Value::minus_infty();
  int last_open = 0;

  for (std::size_t i = 0; i < feasible->size; ++i)
  {
    const lp_interval_t& cur = feasible->intervals[i];
    Value lower(lp_value_new_copy(&cur.a));

    // Trace("cad-check") << "Feasible region: " << lp_interval_to_string(&cur)
    // << std::endl;

    if (lower.get()->type == LP_VALUE_MINUS_INFINITY)
    {
      // Do nothing if we start at -infty.
    }
    else if (last_value < lower)
    {
      // There is an infeasible open interval
      regions.emplace_back(last_value, !last_open, lower, !cur.a_open);
    }
    else if (last_open && cur.a_open && last_value == lower)
    {
      // There is an infeasible point interval
      regions.emplace_back(last_value);
    }
    if (cur.is_point)
    {
      last_value = std::move(lower);
      last_open = true;
    }
    else
    {
      last_value = lp_value_new_copy(&cur.b);
      last_open = cur.b_open;
    }
  }

  if (last_value.get()->type != LP_VALUE_PLUS_INFINITY)
  {
    // Add missing interval to +infty
    regions.emplace_back(last_value, !last_open, Value::plus_infty(), true);
  }

  lp_feasibility_set_delete(feasible);

  return regions;
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
