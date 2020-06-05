
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__CONVERSION_H
#define CVC4__THEORY__NLARITH__LIBPOLY__CONVERSION_H

#include <iostream>

#include <poly/poly.h>
#include <poly/dyadic_rational.h>

#include "integer.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

CVC4::Integer as_cvc_integer(const lp_integer_t* v);
CVC4::Rational as_cvc_rational(const lp_rational_t* v);
CVC4::Rational as_cvc_rational(const lp_dyadic_rational_t* v);

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
