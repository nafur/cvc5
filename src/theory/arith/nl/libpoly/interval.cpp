#include "interval.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

Interval::Interval(const Value& a, bool a_open, const Value& b, bool b_open)
{
  lp_interval_construct(
      &mInterval, a.get(), a_open ? 1 : 0, b.get(), b_open ? 1 : 0);
}

Interval::Interval(const Value& a, const Value& b) : Interval(a, true, b, true)
{
}

Interval::Interval(const Value& a)
{
  lp_interval_construct_point(&mInterval, a.get());
}

Interval::Interval(const Interval& i)
{
  lp_interval_construct_copy(&mInterval, i.get());
}

Interval::~Interval() { lp_interval_destruct(&mInterval); }

Interval& Interval::operator=(Interval i)
{
  std::swap(mInterval, i.mInterval);
  return *this;
}

lp_interval_t* Interval::get() { return &mInterval; }

const lp_interval_t* Interval::get() const { return &mInterval; }

std::ostream& operator<<(std::ostream& os, const Interval& i)
{
  if (i.get()->is_point)
  {
    Assert(!i.get()->a_open && !i.get()->b_open);
    return os << "[ " << lp_value_to_string(&(i.get()->a)) << " ; "
              << lp_value_to_string(&(i.get()->a)) << " ]";
  }
  os << (i.get()->a_open ? "( " : "[ ");
  os << lp_value_to_string(&(i.get()->a)) << " ; "
     << lp_value_to_string(&(i.get()->b));
  os << (i.get()->b_open ? " )" : " ]");
  return os;
}

bool operator==(const Interval& lhs, const Interval& rhs)
{
  // Check lower bound type
  if (lhs.get()->a_open != rhs.get()->a_open) return false;
  // Check point interval flag
  if (lhs.get()->is_point != rhs.get()->is_point) return false;
  if (!lhs.get()->is_point)
  {
    // Check upper bound type (if not a point interval)
    if (lhs.get()->b_open != rhs.get()->b_open) return false;
  }
  const lp_value_t* ll = &(lhs.get()->a);
  const lp_value_t* rl = &(rhs.get()->a);
  // Check lower bound
  if (lp_value_cmp(ll, rl) != 0) return false;
  if (!lhs.get()->is_point)
  {
    const lp_value_t* lu = &(lhs.get()->b);
    const lp_value_t* ru = &(rhs.get()->b);
    // Check lower bound (if not a point interval)
    if (lp_value_cmp(lu, ru) != 0) return false;
  }
  // Identical
  return true;
}

bool operator<(const Interval& lhs, const Interval& rhs)
{
  const lp_value_t* ll = &(lhs.get()->a);
  const lp_value_t* lu = lhs.get()->is_point ? ll : &(lhs.get()->b);
  const lp_value_t* rl = &(rhs.get()->a);
  const lp_value_t* ru = rhs.get()->is_point ? rl : &(rhs.get()->b);

  int lc = lp_value_cmp(ll, rl);
  // Lower bound is smaller
  if (lc < 0) return true;
  // Lower bound is larger
  if (lc > 0) return false;
  // Lower bound type is smaller
  if (!lhs.get()->a_open && rhs.get()->a_open) return true;
  // Lower bound type is larger
  if (lhs.get()->a_open && !rhs.get()->a_open) return false;

  int uc = lp_value_cmp(lu, ru);
  // Upper bound is smaller
  if (uc < 0) return true;
  // Upper bound is larger
  if (uc > 0) return false;
  // Upper bound type is smaller
  if (lhs.get()->b_open && !rhs.get()->b_open) return true;
  // Upper bound type is larger
  if (!lhs.get()->b_open && rhs.get()->b_open) return false;

  // Identical
  return false;
}

bool lower_is_infty(const Interval& i)
{
  return i.get()->a.type == LP_VALUE_MINUS_INFINITY;
}
bool upper_is_infty(const Interval& i)
{
  if (i.get()->is_point) return false;
  return i.get()->b.type == LP_VALUE_PLUS_INFINITY;
}

bool interval_covers(const Interval& lhs, const Interval& rhs)
{
  const lp_value_t* ll = &(lhs.get()->a);
  const lp_value_t* lu = lhs.get()->is_point ? ll : &(lhs.get()->b);
  const lp_value_t* rl = &(rhs.get()->a);
  const lp_value_t* ru = rhs.get()->is_point ? rl : &(rhs.get()->b);

  int lc = lp_value_cmp(ll, rl);
  int uc = lp_value_cmp(lu, ru);

  // Lower bound is smaller and upper bound is larger
  if (lc < 0 && uc > 0) return true;
  // Lower bound is larger or upper bound is smaller
  if (lc > 0 || uc < 0) return false;

  // Now both bounds are identical.
  Assert(lc <= 0 && uc >= 0);
  Assert(lc == 0 || uc == 0);

  // Lower bound is the same and the bound type is stricter
  if (lc == 0 && lhs.get()->a_open && !rhs.get()->a_open) return false;
  // Upper bound is the same and the bound type is stricter
  if (uc == 0 && lhs.get()->b_open && !rhs.get()->b_open) return false;

  // Both bounds are weaker
  return true;
}

bool interval_connect(const Interval& lhs, const Interval& rhs)
{
  Assert(lhs < rhs) << "Can only check for a connection if lhs < rhs.";
  const lp_value_t* lu =
      lhs.get()->is_point ? &(lhs.get()->a) : &(lhs.get()->b);
  const lp_value_t* rl = &(rhs.get()->a);
  int c = lp_value_cmp(lu, rl);
  if (c < 0) return false;
  if (c > 0) return true;
  Assert(c == 0);
  if (lhs.get()->is_point || rhs.get()->is_point || !lhs.get()->b_open
      || !rhs.get()->a_open) {
    Trace("libpoly::interval_connect") << lhs << " and " << rhs << " touch and the intermediate point is covered." << std::endl;
    return true;
  }
  return false;
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4