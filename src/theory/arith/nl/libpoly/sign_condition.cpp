
#include "sign_condition.h"

#include "base/check.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

lp_sign_condition_t to_sign_condition(SignCondition sc)
{
  return static_cast<lp_sign_condition_t>(sc);
}

std::ostream& operator<<(std::ostream& os, const SignCondition& sc)
{
  switch (sc)
  {
    case SignCondition::LT: os << "<"; break;
    case SignCondition::LE: os << "<="; break;
    case SignCondition::EQ: os << "="; break;
    case SignCondition::NE: os << "!="; break;
    case SignCondition::GT: os << ">"; break;
    case SignCondition::GE: os << ">="; break;
  }
  return os;
}

bool evaluate_sign_condition(SignCondition sc, int sgn)
{
  Assert(-1 <= sgn && sgn <= 1) << "sgn should be -1, 0 or 1.";
  return lp_sign_condition_consistent(to_sign_condition(sc), sgn);
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
