
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__SIGN_CONDITION_H
#define CVC4__THEORY__NLARITH__LIBPOLY__SIGN_CONDITION_H

#include "base/check.h"

#include <poly/sign_condition.h>

#include <iostream>

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

enum class SignCondition {
  LT, LE, EQ, NE, GT, GE
};

inline lp_sign_condition_t to_sign_condition(SignCondition sc) {
  switch (sc) {
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

/** Stream the given SignCondition to an output stream. */
inline std::ostream& operator<<(std::ostream& os, const SignCondition& sc)
{
    switch (sc) {
        case SignCondition::LT: os << "<"; break;
        case SignCondition::LE: os << "<="; break;
        case SignCondition::EQ: os << "="; break;
        case SignCondition::NE: os << "!="; break;
        case SignCondition::GT: os << ">"; break;
        case SignCondition::GE: os << ">="; break;
    }
  return os;
}

/** Evaluates a sign condition.
 * Expected values of sgn: -1, 0, 1
 */
inline bool evaluate_sign_condition(SignCondition sc, int sgn) {
  Assert(-1 <= sgn && sgn <= 1) << "sgn should be -1, 0 or 1.";
  switch (sc) {
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

#endif
