
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__VARIABLE_H
#define CVC4__THEORY__NLARITH__LIBPOLY__VARIABLE_H

#include <poly/variable_db.h>

#include <iostream>

#include "utils.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/**
 * Implements a wrapper for lp_variable_t from libpoly.
 */
class Variable
{
  /** The actual variable. */
  lp_variable_t mVariable;

 public:
  /** Construct with a null variable. */
  Variable();
  /** Construct from a lp_variable_t. */
  Variable(lp_variable_t var);
  /** Construct a new variable with the given name. */
  Variable(const char* name);

  /** Get the internal lp_variable_t. Note that it's only a type alias for long.
   */
  lp_variable_t get() const;
};

/** Stream the given Variable to an output stream. */
std::ostream& operator<<(std::ostream& os, const Variable& v);

/** Compare two variables for equality. */
bool operator==(const Variable& lhs, const Variable& rhs);
/** Compare two variables for inequality. */
bool operator!=(const Variable& lhs, const Variable& rhs);

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
