#include "poly_util.h"

#include "base/check.h"
#include "maybe.h"
#include "poly/polyxx.h"
#include "poly/polyxx/upolynomial_roots.h"
#include "util/integer.h"
#include "util/rational.h"
#include "util/real_algebraic_number.h"

#include <map>

namespace CVC4 {
namespace poly_utils {

Integer to_integer(const poly::Integer& i) {
#ifdef CVC4_GMP_IMP
  return Integer(*poly::detail::cast_to_gmp(&i));
#endif
#ifdef CVC4_CLN_IMP
  Assert(false) << "This is not yet implemented";
  return poly::Integer();
#endif
}
Rational to_rational(const poly::Rational& r) {
#ifdef CVC4_GMP_IMP
  return Rational(*poly::detail::cast_to_gmp(&r));
#endif
#ifdef CVC4_CLN_IMP
  Assert(false) << "This is not yet implemented";
  return poly::Integer();
#endif
}
Rational to_rational(const poly::DyadicRational& dr) {
  return Rational(to_integer(numerator(dr)), to_integer(denominator(dr)));
}

poly::Integer to_integer(const Integer& i)
{
#ifdef CVC4_GMP_IMP
  return poly::Integer(i.getValue());
#endif
#ifdef CVC4_CLN_IMP
  Assert(false) << "This is not yet implemented";
  return poly::Integer();
#endif
}
std::vector<poly::Integer> to_integer(const std::vector<Integer>& vi)
{
  std::vector<poly::Integer> res;
  for (const auto& i : vi) res.emplace_back(to_integer(i));
  return res;
}
poly::Rational to_rational(const Rational& r)
{
#ifdef CVC4_GMP_IMP
  return poly::Rational(r.getValue());
#endif
#ifdef CVC4_CLN_IMP
  Assert(false) << "This is not yet implemented";
  return poly::Rational();
#endif
}

Maybe<poly::DyadicRational> to_dyadic_rational(const Rational& r)
{
  Integer den = r.getDenominator();
  if (den.isOne())
  {
    return poly::DyadicRational(poly::Integer(r.getNumerator().getValue()));
  }
  unsigned long exp = den.isPow2();
  if (exp > 0)
  {
    return div_2exp(
        poly::DyadicRational(poly::Integer(r.getNumerator().getValue())), exp);
  }
  return Maybe<poly::DyadicRational>();
}
Maybe<poly::DyadicRational> to_dyadic_rational(const poly::Rational& r)
{
  poly::Integer den = denominator(r);
  if (den == poly::Integer(1))
  {
    return poly::DyadicRational(numerator(r));
  }
  unsigned long size = bit_size(den) - 1;
  std::cout << "Testing whether " << den << " == 2^" << size << std::endl;
  if (mul_pow2(poly::Integer(1), size) == den)
  {
    return div_2exp(poly::DyadicRational(numerator(r)), size);
  }
  return Maybe<poly::DyadicRational>();
}

/**
 * Assuming that r is dyadic, makes one refinement step to move r closer to
 * original.
 */
void approximate_to_dyadic(poly::Rational& r, const poly::Rational& original)
{
  poly::Integer n = mul_pow2(numerator(r), 1);
  if (r < original)
  {
    ++n;
  }
  else if (r > original)
  {
    --n;
  }
  r = poly::Rational(n, mul_pow2(denominator(r), 1));
}

RealAlgebraicNumber from_rationals_with_refinement(poly::UPolynomial&& p,
                                                   const Rational& lower,
                                                   const Rational upper)
{
  poly::Rational origl(lower.getValue());
  poly::Rational origu(upper.getValue());
  poly::Rational l(floor(origl));
  poly::Rational u(ceil(origu));
  poly::RationalInterval ri(l, u);
  while (count_real_roots(p, ri) != 1)
  {
    approximate_to_dyadic(l, origl);
    approximate_to_dyadic(u, origu);
    ri = poly::RationalInterval(l, u);
  }
  Assert(count_real_roots(p, poly::RationalInterval(l, u)) == 1);
  auto ml = to_dyadic_rational(l);
  auto mu = to_dyadic_rational(u);
  Assert(ml && mu) << "Both bounds should be dyadic by now.";
  return RealAlgebraicNumber(poly::AlgebraicNumber(
      std::move(p), poly::DyadicInterval(ml.value(), mu.value())));
}

}  // namespace poly_utils
}  // namespace CVC4
