#include "conversion.h"

#include <poly/dyadic_rational.h>
#include <poly/integer.h>
#include <poly/rational.h>

#include "util/integer.h"
#include "util/rational.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

CVC4::Integer as_cvc_integer(const lp_integer_t* v) {
  // TODO(Gereon): Make this conversion more efficient.
  std::string tmp(lp_integer_to_string(v));
  return CVC4::Integer(tmp);
}
CVC4::Rational as_cvc_rational(const lp_rational_t* v) {
  // TODO(Gereon): Make this conversion more efficient.
  std::string tmp(lp_rational_to_string(v));
  return CVC4::Rational(tmp);
}
CVC4::Rational as_cvc_rational(const lp_dyadic_rational_t* v) {
  // TODO(Gereon): Make this conversion more efficient.
  std::string tmp(lp_dyadic_rational_to_string(v));
  return CVC4::Rational(tmp);
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
