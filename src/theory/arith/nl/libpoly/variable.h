
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
  friend std::ostream& operator<<(std::ostream& os, const Variable& v);
  /** The actual variable. */
  lp_variable_t mVariable;

 public:
  /** Construct a new variable with the given name. */
  Variable(const char* name)
      : mVariable(lp_variable_db_new_variable(variable_db.get(), name))
  {
  }

  /** Get the internal lp_variable_t. Note that it's only a type alias for long.
   */
  lp_variable_t get() const { return mVariable; }
};

/** Stream the given Variable to an output stream. */
inline std::ostream& operator<<(std::ostream& os, const Variable& v)
{
  return os << lp_variable_db_get_name(variable_db.get(), v.get());
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
