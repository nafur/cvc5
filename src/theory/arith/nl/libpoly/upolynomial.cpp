#include "upolynomial.h"

#include <poly/upolynomial_factors.h>

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/** A deleter for an std::unique_ptr holding a lp_upolynomial_t pointer */
inline void upolynomial_deleter(lp_upolynomial_t* ptr)
{
  lp_upolynomial_delete(ptr);
}
/** Create from a lp_upolynomial_t pointer, claiming it's ownership. */
UPolynomial::UPolynomial(lp_upolynomial_t* poly)
    : mPoly(poly, upolynomial_deleter)
{
}
/** Create from a degree and a list of coefficients. */
UPolynomial::UPolynomial(std::size_t degree, const int* coeffs)
    : mPoly(lp_upolynomial_construct_from_int(lp_Z, degree, coeffs),
            upolynomial_deleter)
{
}
/** Create from a degree and a list of coefficients. */
UPolynomial::UPolynomial(std::size_t degree, const lp_integer_t* coeffs)
    : mPoly(lp_upolynomial_construct(lp_Z, degree, coeffs), upolynomial_deleter)
{
}
/** Copy from the given UPolynomial. */
UPolynomial::UPolynomial(const UPolynomial& poly)
    : mPoly(lp_upolynomial_construct_copy(poly.get()), upolynomial_deleter)
{
}

/** Get a non-const pointer to the internal lp_upolynomial_t. Handle with care!
 */
lp_upolynomial_t* UPolynomial::get() { return mPoly.get(); }
/** Get a const pointer to the internal lp_upolynomial_t. */
const lp_upolynomial_t* UPolynomial::get() const { return mPoly.get(); }
/** Release the lp_upolynomial_t pointer. This yields ownership of the returned
 * pointer. */
lp_upolynomial_t* UPolynomial::release() { return mPoly.release(); }

std::ostream& operator<<(std::ostream& os, const UPolynomial& p)
{
  return os << lp_upolynomial_to_string(p.get());
}

UPolynomial operator+(const UPolynomial& lhs, const UPolynomial& rhs)
{
  return UPolynomial(lp_upolynomial_add(lhs.get(), rhs.get()));
}
UPolynomial operator-(const UPolynomial& lhs, const UPolynomial& rhs)
{
  return UPolynomial(lp_upolynomial_sub(lhs.get(), rhs.get()));
}
UPolynomial operator*(const UPolynomial& lhs, const UPolynomial& rhs)
{
  return UPolynomial(lp_upolynomial_mul(lhs.get(), rhs.get()));
}

std::size_t degree(const UPolynomial& p)
{
  return lp_upolynomial_degree(p.get());
}

std::vector<UPolynomial> square_free_factors(const UPolynomial& p,
                                             bool with_constant)
{
  auto factors = lp_upolynomial_factor_square_free(p.get());
  std::vector<UPolynomial> res;

  if (with_constant)
  {
    res.emplace_back(0, lp_upolynomial_factors_get_constant(factors));
  }

  for (std::size_t i = 0; i < lp_upolynomial_factors_size(factors); ++i)
  {
    std::size_t multiplicity = 0;
    res.emplace_back(
        lp_upolynomial_factors_get_factor(factors, i, &multiplicity));
  }

  lp_upolynomial_factors_destruct(factors, 0);
  return res;
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
