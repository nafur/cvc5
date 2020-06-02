#include "ran.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

std::vector<RAN> isolate_real_roots(const UPolynomial& p)
{
  lp_algebraic_number_t* roots = new lp_algebraic_number_t[degree(p)];
  std::size_t roots_size;
  lp_upolynomial_roots_isolate(p.get(), roots, &roots_size);
  std::vector<RAN> res;
  for (std::size_t i = 0; i < roots_size; ++i)
  {
    res.emplace_back(roots[i]);
  }
  for (std::size_t i = 0; i < roots_size; ++i)
  {
    lp_algebraic_number_destruct(&roots[i]);
  }
  delete[] roots;
  return res;
}

std::vector<Value> isolate_real_roots(const Polynomial& p, const Assignment& a)
{
  lp_value_t* roots = new lp_value_t[degree(p)];
  std::size_t roots_size;
  lp_polynomial_roots_isolate(p.get(), a.get(), roots, &roots_size);
  std::vector<Value> res;
  for (std::size_t i = 0; i < roots_size; ++i)
  {
    res.emplace_back(lp_value_new_copy(&roots[i]));
  }
  for (std::size_t i = 0; i < roots_size; ++i)
  {
    lp_value_destruct(&roots[i]);
  }
  delete[] roots;
  return res;
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
