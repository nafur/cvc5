
#include "sign_condition.h"

#include "base/check.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

lp_sign_condition_t to_sign_condition(SignCondition sc)
{
  switch (sc)
  {
    case SignCondition::LT: return LP_SGN_LT_0;
    case SignCondition::LE: return LP_SGN_LE_0;
    case SignCondition::EQ: return LP_SGN_EQ_0;
    case SignCondition::NE: return LP_SGN_NE_0;
    case SignCondition::GT: return LP_SGN_GT_0;
    case SignCondition::GE: return LP_SGN_GE_0;
  }
  Assert(false);
  return LP_SGN_EQ_0;
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
  switch (sc)
  {
    case SignCondition::LT: return sgn < 0;
    case SignCondition::LE: return sgn <= 0;
    case SignCondition::EQ: return sgn == 0;
    case SignCondition::NE: return sgn != 0;
    case SignCondition::GT: return sgn > 0;
    case SignCondition::GE: return sgn >= 0;
  }
  Assert(false);
  return false;
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
