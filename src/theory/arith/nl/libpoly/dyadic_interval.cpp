#include "dyadic_interval.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

DyadicInterval::DyadicInterval(const Integer& a, const Integer& b)
{
  lp_dyadic_interval_construct_from_integer(&mInterval, a.get(), 1, b.get(), 1);
}

DyadicInterval::DyadicInterval(const DyadicInterval& i)
{
  lp_dyadic_interval_construct_copy(&mInterval, i.get());
}

DyadicInterval::~DyadicInterval() { lp_dyadic_interval_destruct(&mInterval); }

DyadicInterval& DyadicInterval::operator=(DyadicInterval i)
{
  std::swap(mInterval, i.mInterval);
  return *this;
}

lp_dyadic_interval_t* DyadicInterval::get() { return &mInterval; }

const lp_dyadic_interval_t* DyadicInterval::get() const { return &mInterval; }

std::ostream& operator<<(std::ostream& os, const DyadicInterval& i)
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