#include "integer.h"

#include <iostream>

#include "util/integer.h"
#include "utils.h"
#include "value.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

Integer::Integer() { lp_integer_construct(&mInt); }

Integer::Integer(long i) { lp_integer_construct_from_int(lp_Z, &mInt, i); }

Integer::Integer(const Integer& i)
{
  lp_integer_construct_copy(lp_Z, &mInt, i.get());
}

#ifdef CVC4_GMP_IMP
Integer::Integer(const mpz_class& m)
{
  lp_integer_construct_copy(lp_Z, &mInt, m.get_mpz_t());
}
#endif
#ifdef CVC4_CLN_IMP
Integer::Integer(const cln::cl_I& i)
{
  // TODO(Gereon): Check whether we can do better when converting from cln::cl_I
  // to mpz_t
  if (std::numeric_limits<long>::min() <= i
      && i <= std::numeric_limits<long>::max())
  {
    lp_integer_construct_from_int(lp_Z, &mInt, cln::cl_I_to_long(i));
  }
  std::stringstream s;
  s << i;
  mpz_t tmp;
  mpz_set_str(tmp, s.str().c_str(), 0);
  lp_integer_construct_copy(lp_Z, &mInt, tmp);
}
#endif

Integer::~Integer() { lp_integer_destruct(&mInt); }

Integer& Integer::operator=(Integer i)
{
  std::swap(mInt, i.mInt);
  return *this;
}

Integer::operator Value() const
{
  return Value(lp_value_new(lp_value_type_t::LP_VALUE_INTEGER, &mInt));
}

lp_integer_t* Integer::get() { return &mInt; }

const lp_integer_t* Integer::get() const { return &mInt; }

std::ostream& operator<<(std::ostream& os, const Integer& i)
{
  return os << lp_integer_to_string(i.get());
}

Integer operator-(const Integer& i)
{
  Integer res;
  lp_integer_neg(lp_Z, res.get(), i.get());
  return res;
}

Integer& operator*=(Integer& lhs, const Integer& rhs)
{
  lp_integer_mul(lp_Z, lhs.get(), lhs.get(), rhs.get());
  return lhs;
}

Integer& operator/=(Integer& lhs, const Integer& rhs)
{
  lp_integer_div_exact(lp_Z, lhs.get(), lhs.get(), rhs.get());
  return lhs;
}

Integer gcd(const Integer& a, const Integer& b)
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
