#include "upolynomial.h"

#include <poly/upolynomial_factors.h>

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

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
