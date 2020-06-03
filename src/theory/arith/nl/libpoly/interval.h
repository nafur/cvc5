
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__INTERVAL_H
#define CVC4__THEORY__NLARITH__LIBPOLY__INTERVAL_H

#include "integer.h"
#include "utils.h"
#include "value.h"

#include "base/check.h"
#include "base/output.h"

#include <poly/interval.h>

#include <iostream>

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
  friend std::ostream& operator<<(std::ostream& os, const Interval& i);

 private:
  /** The actual interval. */
  lp_interval_t mInterval;

 public:
  /** Disallow the default constructor. */
  Interval() = delete;
  /** Construct an open interval from the given two values and bound types. */
  Interval(const Value& a, bool a_open, const Value& b, bool b_open)
  {
    lp_interval_construct(
        &mInterval, a.get(), a_open ? 1 : 0, b.get(), b_open ? 1 : 0);
  }
  /** Construct an open interval from the given two values. */
  Interval(const Value& a, const Value& b): Interval(a, true, b, true) {}
  /** Construct a point interval from the given value. */
  Interval(const Value& a) {
    lp_interval_construct_point(&mInterval, a.get());
  }
  /** Copy from the given Interval. */
  Interval(const Interval& i)
  {
    lp_interval_construct_copy(&mInterval, i.get());
  }
  /** Custom destructor. */
  ~Interval() { lp_interval_destruct(&mInterval); }
  /** Assign from the given Interval. */
  Interval& operator=(Interval i)
  {
    std::swap(mInterval, i.mInterval);
    return *this;
  }

  /** Get a non-const pointer to the internal lp_interval_t. Handle with
   * care! */
  lp_interval_t* get() { return &mInterval; }
  /** Get a const pointer to the internal lp_interval_t. */
  const lp_interval_t* get() const { return &mInterval; }
};

/** Stream the given Interval to an output stream. */
std::ostream& operator<<(std::ostream& os, const Interval& i);

bool operator==(const Interval& lhs, const Interval& rhs);

bool operator<(const Interval& lhs, const Interval& rhs);

inline bool lower_is_infty(const Interval& i) {
  return i.get()->a.type == LP_VALUE_MINUS_INFINITY;
}
inline bool upper_is_infty(const Interval& i) {
  if (i.get()->is_point) return false;
  return i.get()->b.type == LP_VALUE_PLUS_INFINITY;
}

bool interval_covers(const Interval& lhs, const Interval& rhs);
bool interval_connect(const Interval& lhs, const Interval& rhs);

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
