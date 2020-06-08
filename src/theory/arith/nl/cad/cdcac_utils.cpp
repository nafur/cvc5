#include "cdcac_utils.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

using namespace libpoly;

/** Induces an ordering on libpoly intervals that is suitable for redundancy
 * removal as implemented in clean_intervals.
 */
inline bool compare_for_cleanup(const Interval& lhs, const Interval& rhs)
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

  // Attention: Here it differs from the regular interval ordering!
  int uc = lp_value_cmp(lu, ru);
  // Upper bound is smaller
  if (uc < 0) return false;
  // Upper bound is larger
  if (uc > 0) return true;
  // Upper bound type is smaller
  if (lhs.get()->b_open && !rhs.get()->b_open) return false;
  // Upper bound type is larger
  if (!lhs.get()->b_open && rhs.get()->b_open) return true;

  // Identical
  return false;
}

void clean_intervals(std::vector<CACInterval>& intervals)
{
  // Simplifies removal of redundancies later on.
  if (intervals.size() < 2) return;

  // Sort intervals.
  std::sort(intervals.begin(),
            intervals.end(),
            [](const CACInterval& lhs, const CACInterval& rhs) {
              return compare_for_cleanup(lhs.mInterval, rhs.mInterval);
            });

  // Remove intervals that are covered by others.
  // Implementation roughly follows
  // https://en.cppreference.com/w/cpp/algorithm/remove Find first interval that
  // covers the next one.
  std::size_t first = 0;
  for (; first < intervals.size() - 1; ++first)
  {
    if (interval_covers(intervals[first].mInterval,
                        intervals[first + 1].mInterval))
    {
      break;
    }
  }
  // If such an interval exists, remove accordingly.
  if (first < intervals.size() - 1)
  {
    for (std::size_t i = first + 2; i < intervals.size(); ++i)
    {
      if (!interval_covers(intervals[first].mInterval, intervals[i].mInterval))
      {
        // Interval is not covered. Move it to the front and bump front.
        ++first;
        intervals[first] = std::move(intervals[i]);
      }
      // Else: Interval is covered as well.
    }
    // Erase trailing values
    while (intervals.size() > first + 1)
    {
      intervals.pop_back();
    }
  }
}

std::vector<Node> collect_constraints(const std::vector<CACInterval>& intervals)
{
  std::vector<Node> res;
  for (const auto& i : intervals)
  {
    res.insert(res.end(), i.mOrigins.begin(), i.mOrigins.end());
  }
  std::sort(res.begin(), res.end());
  auto it = std::unique(res.begin(), res.end());
  res.erase(it, res.end());
  return res;
}

bool sample_outside(const std::vector<CACInterval>& infeasible, Value& sample)
{
  if (infeasible.empty())
  {
    sample = Integer(0);
    return true;
  }
  if (!lower_is_infty(infeasible.front().mInterval))
  {
    Trace("cdcac") << "Sample before " << infeasible.front().mInterval
                   << std::endl;
    const auto* i = infeasible.front().mInterval.get();
    sample =
        sample_between(Value::minus_infty().get(), true, &i->a, !i->a_open);
    return true;
  }
  for (std::size_t i = 0; i < infeasible.size() - 1; ++i)
  {
    if (!interval_connect(infeasible[i].mInterval, infeasible[i + 1].mInterval))
    {
      const auto* l = infeasible[i].mInterval.get();
      const auto* r = infeasible[i + 1].mInterval.get();

      Trace("cdcac") << "Sample between " << infeasible[i].mInterval << " and "
                     << infeasible[i + 1].mInterval << std::endl;

      if (l->is_point)
      {
        sample = sample_between(&l->a, true, &r->a, !r->a_open);
      }
      else
      {
        sample = sample_between(&l->b, !l->b_open, &r->a, !r->a_open);
      }
      return true;
    }
    else
    {
      Trace("cdcac") << infeasible[i].mInterval << " and "
                     << infeasible[i + 1].mInterval << " connect" << std::endl;
    }
  }
  if (!upper_is_infty(infeasible.back().mInterval))
  {
    Trace("cdcac") << "Sample above " << infeasible.back().mInterval
                   << std::endl;
    const auto* i = infeasible.back().mInterval.get();
    if (i->is_point)
    {
      sample = sample_between(&i->a, true, Value::plus_infty().get(), true);
    }
    else
    {
      sample =
          sample_between(&i->b, !i->b_open, Value::plus_infty().get(), true);
    }
    return true;
  }
  return false;
}

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4