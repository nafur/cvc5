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
  friend std::ostream& operator<<(std::ostream& os, const Integer& i);
  /** The actual integer. */
  lp_integer_t mInt;

 public:
  /** Construct a zero integer. */
  Integer() { lp_integer_construct(&mInt); }
  /** Construct from the given integer. */
  Integer(long i) { lp_integer_construct_from_int(lp_Z, &mInt, i); }
  /** Copy from the given Integer. */
  Integer(const Integer& i) { lp_integer_construct_copy(lp_Z, &mInt, i.get()); }
#ifdef CVC4_GMP_IMP
  /** Constructs from a gmp integer. */
  Integer(const mpz_class& m)
  {
    lp_integer_construct_copy(lp_Z, &mInt, m.get_mpz_t());
  }
#endif
#ifdef CVC4_CLN_IMP
  /** Constructs from a cln integer. */
  Integer(const cln::cl_I& i)
  {
    // TODO(Gereon): Check whether we can do better when converting from cln::cl_I to mpz_t
    if (std::numeric_limits<long>::min() <= i && i <= std::numeric_limits<long>::max()) {
      lp_integer_construct_from_int(lp_Z, &mInt, cln::cl_I_to_long(i));
    }
    std::stringstream s;
    s << i;
    mpz_t tmp;
    mpz_set_str(tmp, s.str().c_str(), 0);
    lp_integer_construct_copy(lp_Z, &mInt, tmp);
  }
#endif
  /** Custom destructor. */
  ~Integer() { lp_integer_destruct(&mInt); }
  /** Assign from the given Integer. */
  Integer& operator=(Integer i)
  {
    std::swap(mInt, i.mInt);
    return *this;
  }

  /** Implicitly convert to a Value. */
  operator Value() const
  {
    return Value(lp_value_new(lp_value_type_t::LP_VALUE_INTEGER, &mInt));
  }

  /** Get a non-const pointer to the internal lp_integer_t. Handle with care! */
  lp_integer_t* get() { return &mInt; }
  /** Get a const pointer to the internal lp_integer_t. */
  const lp_integer_t* get() const { return &mInt; }
};
/** Stream the given Integer to an output stream. */
inline std::ostream& operator<<(std::ostream& os, const Integer& i)
{
  return os << lp_integer_to_string(i.get());
}

/** Unary negation for an Integer. */
inline Integer operator-(const Integer& i)
{
  Integer res;
  lp_integer_neg(lp_Z, res.get(), i.get());
  return res;
}

/** Multiply and assign two Integers. */
inline Integer& operator*=(Integer& lhs, const Integer& rhs)
{
  lp_integer_mul(lp_Z, lhs.get(), lhs.get(), rhs.get());
  return lhs;
}

/** Divide and assign two Integers. Assumes the division is exact! */
inline Integer& operator/=(Integer& lhs, const Integer& rhs)
{
    lp_integer_div_exact(lp_Z, lhs.get(), lhs.get(), rhs.get());
  return lhs;
}

/** Compute the GCD of two Integers. */
inline Integer gcd(const Integer& a, const Integer& b)
{
  Integer res;
  lp_integer_gcd_Z(res.get(), a.get(), b.get());
  return res;
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
