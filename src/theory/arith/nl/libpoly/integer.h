#ifndef CVC4__THEORY__NLARITH__LIBPOLY__INTEGER_H
#define CVC4__THEORY__NLARITH__LIBPOLY__INTEGER_H

#include <poly/integer.h>

#include <iostream>

#include "util/integer.h"
#include "utils.h"
#include "value.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/**
 * Implements a wrapper for lp_integer_t from libpoly.
 */
class Integer
{
  /** The actual integer. */
  lp_integer_t mInt;

 public:
  /** Construct a zero integer. */
  Integer();
  /** Construct from the given integer. */
  Integer(long i);
  /** Copy from the given Integer. */
  Integer(const Integer& i);
#ifdef CVC4_GMP_IMP
  /** Constructs from a gmp integer. */
  Integer(const mpz_class& m);
#endif
#ifdef CVC4_CLN_IMP
  /** Constructs from a cln integer. */
  Integer(const cln::cl_I& i);
#endif
  /** Custom destructor. */
  ~Integer();
  /** Assign from the given Integer. */
  Integer& operator=(Integer i);

  /** Implicitly convert to a Value. */
  operator Value() const;

  /** Get a non-const pointer to the internal lp_integer_t. Handle with care! */
  lp_integer_t* get();
  /** Get a const pointer to the internal lp_integer_t. */
  const lp_integer_t* get() const;
};
/** Stream the given Integer to an output stream. */
std::ostream& operator<<(std::ostream& os, const Integer& i);

/** Unary negation for an Integer. */
Integer operator-(const Integer& i);

/** Multiply and assign two Integers. */
Integer& operator*=(Integer& lhs, const Integer& rhs);

/** Divide and assign two Integers. Assumes the division is exact! */
Integer& operator/=(Integer& lhs, const Integer& rhs);

/** Compute the GCD of two Integers. */
Integer gcd(const Integer& a, const Integer& b);

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
