
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__ASSIGNMENT_H
#define CVC4__THEORY__NLARITH__LIBPOLY__ASSIGNMENT_H

#include <poly/assignment.h>

#include <iostream>

#include "utils.h"
#include "value.h"
#include "variable.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/**
 * Implements a wrapper for lp_assignment_t from libpoly.
 */
class Assignment
{
  /** The actual assignment. */
  deleting_unique_ptr<lp_assignment_t> mAssignment;

 public:
  /** Construct an empty assignment. */
  Assignment();

  /** Get a non-const pointer to the internal lp_assignment_t. Handle with care!
   */
  lp_assignment_t* get();
  /** Get a const pointer to the internal lp_assignment_t. */
  const lp_assignment_t* get() const;

  /** Assign var to the given value. */
  void set(const Variable& var, const Value& value);
  /** Unassign the given variable. */
  void unset(const Variable& var);
  /** Clear the assignment. */
  void clear();
};
/** Stream the given Assignment to an output stream. */
std::ostream& operator<<(std::ostream& os, const Assignment& a);

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
