
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__INTERVAL_H
#define CVC4__THEORY__NLARITH__LIBPOLY__INTERVAL_H

#include <poly/dyadic_interval.h>

#include <iostream>

#include "integer.h"
#include "utils.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/**
 * Implements a wrapper for lp_dyadic_interval_t from libpoly.
 */
class Interval
{
  friend std::ostream& operator<<(std::ostream& os, const Interval& i);

 private:
  /** The actual interval. */
  lp_dyadic_interval_t mInterval;

 public:
  /** Disallow the default constructor. */
  Interval() = delete;
  /** Construct an open interval from the given two intervals. */
  Interval(const Integer& a, const Integer& b)
  {
    lp_dyadic_interval_construct_from_integer(
        &mInterval, a.get(), 1, b.get(), 1);
  }
  /** Copy from the given Interval. */
  Interval(const Interval& i)
  {
    lp_dyadic_interval_construct_copy(&mInterval, i.get());
  }
  /** Custom destructor. */
  ~Interval() { lp_dyadic_interval_destruct(&mInterval); }
  /** Assign from the given Interval. */
  Interval& operator=(Interval i)
  {
    std::swap(mInterval, i.mInterval);
    return *this;
  }

  /** Get a non-const pointer to the internal lp_dyadic_interval_t. Handle with
   * care! */
  lp_dyadic_interval_t* get() { return &mInterval; }
  /** Get a const pointer to the internal lp_dyadic_interval_t. */
  const lp_dyadic_interval_t* get() const { return &mInterval; }
};

/** Stream the given Interval to an output stream. */
inline std::ostream& operator<<(std::ostream& os, const Interval& i)
{
  os << (i.get()->a_open ? "( " : "[ ");
  os << lp_dyadic_rational_to_string(&(i.get()->a)) << " ; "
     << lp_dyadic_rational_to_string(&(i.get()->b));
  os << (i.get()->b_open ? " )" : " ]");
  return os;
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
