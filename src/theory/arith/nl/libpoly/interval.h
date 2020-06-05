
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__INTERVAL_H
#define CVC4__THEORY__NLARITH__LIBPOLY__INTERVAL_H

#include <poly/interval.h>

#include <iostream>

#include "value.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/**
 * Implements a wrapper for lp_interval_t from libpoly.
 */
class Interval
{
 private:
  /** The actual interval. */
  lp_interval_t mInterval;

 public:
  /** Disallow the default constructor. */
  Interval() = delete;
  /** Construct an open interval from the given two values and bound types. */
  Interval(const Value& a, bool a_open, const Value& b, bool b_open);
  /** Construct an open interval from the given two values. */
  Interval(const Value& a, const Value& b);
  /** Construct a point interval from the given value. */
  Interval(const Value& a);
  /** Copy from the given Interval. */
  Interval(const Interval& i);
  /** Custom destructor. */
  ~Interval();
  /** Assign from the given Interval. */
  Interval& operator=(Interval i);

  /** Get a non-const pointer to the internal lp_interval_t. Handle with
   * care! */
  lp_interval_t* get();
  /** Get a const pointer to the internal lp_interval_t. */
  const lp_interval_t* get() const;
};

/** Stream the given Interval to an output stream. */
std::ostream& operator<<(std::ostream& os, const Interval& i);

bool operator==(const Interval& lhs, const Interval& rhs);

bool operator<(const Interval& lhs, const Interval& rhs);

bool lower_is_infty(const Interval& i);
bool upper_is_infty(const Interval& i);

bool interval_covers(const Interval& lhs, const Interval& rhs);
bool interval_connect(const Interval& lhs, const Interval& rhs);

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
