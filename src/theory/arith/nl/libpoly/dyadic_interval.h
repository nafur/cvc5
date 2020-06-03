
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__DYADIC_INTERVAL_H
#define CVC4__THEORY__NLARITH__LIBPOLY__DYADIC_INTERVAL_H

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
class DyadicInterval
{
  friend std::ostream& operator<<(std::ostream& os, const DyadicInterval& i);

 private:
  /** The actual interval. */
  lp_dyadic_interval_t mInterval;

 public:
  /** Disallow the default constructor. */
  DyadicInterval() = delete;
  /** Construct an open interval from the given two integers. */
  DyadicInterval(const Integer& a, const Integer& b)
  {
    lp_dyadic_interval_construct_from_integer(
        &mInterval, a.get(), 1, b.get(), 1);
  }
  /** Copy from the given DyadicInterval. */
  DyadicInterval(const DyadicInterval& i)
  {
    lp_dyadic_interval_construct_copy(&mInterval, i.get());
  }
  /** Custom destructor. */
  ~DyadicInterval() { lp_dyadic_interval_destruct(&mInterval); }
  /** Assign from the given DyadicInterval. */
  DyadicInterval& operator=(DyadicInterval i)
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

/** Stream the given DyadicInterval to an output stream. */
inline std::ostream& operator<<(std::ostream& os, const DyadicInterval& i)
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
