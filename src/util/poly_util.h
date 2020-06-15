#include "cvc4_public.h"

#ifndef CVC4__POLY_UTIL_H
#define CVC4__POLY_UTIL_H

#include "maybe.h"
#include "poly/polyxx.h"
#include "util/integer.h"
#include "util/rational.h"
#include "util/real_algebraic_number.h"

#include <map>

namespace CVC4 {
namespace poly_utils {

Integer to_integer(const poly::Integer& i);
Rational to_rational(const poly::Rational& r);
Rational to_rational(const poly::DyadicRational& dr);

poly::Integer to_integer(const Integer& i);
std::vector<poly::Integer> to_integer(const std::vector<Integer>& vi);
poly::Rational to_rational(const Rational& r);
Maybe<poly::DyadicRational> to_dyadic_rational(const Rational& r);
Maybe<poly::DyadicRational> to_dyadic_rational(const poly::Rational& r);


/**
 * Assuming that r is dyadic, makes one refinement step to move r closer to
 * original.
 * Best start with lower(original) or ceil(original).
 */
void approximate_to_dyadic(poly::Rational& r, const poly::Rational& original);

RealAlgebraicNumber from_rationals_with_refinement(poly::UPolynomial&& p,
                                                   const Rational& lower,
                                                   const Rational upper);

}  // namespace poly_utils
}  // namespace CVC4

#endif /* CVC4__POLY_UTIL_H */
