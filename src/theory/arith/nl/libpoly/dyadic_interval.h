
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
 private:
  /** The actual interval. */
  lp_dyadic_interval_t mInterval;

 public:
  /** Disallow the default constructor. */
  DyadicInterval() = delete;
  /** Construct an open interval from the given two integers. */
  DyadicInterval(const Integer& a, const Integer& b);
  /** Copy from the given DyadicInterval. */
  DyadicInterval(const DyadicInterval& i);
  /** Custom destructor. */
  ~DyadicInterval();
  /** Assign from the given DyadicInterval. */
  DyadicInterval& operator=(DyadicInterval i);

  /** Get a non-const pointer to the internal lp_dyadic_interval_t. Handle with
   * care! */
  lp_dyadic_interval_t* get();
  /** Get a const pointer to the internal lp_dyadic_interval_t. */
  const lp_dyadic_interval_t* get() const;
};

/** Stream the given DyadicInterval to an output stream. */
std::ostream& operator<<(std::ostream& os, const DyadicInterval& i);

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
