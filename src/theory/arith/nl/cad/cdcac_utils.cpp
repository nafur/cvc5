#include "cdcac_utils.h"

#include <fstream>

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

void render(std::ostream& os, const Value& val, bool approx_from_below = true) {
  const lp_value_t* v = val.get();
  if (v->type == LP_VALUE_INTEGER) {
    if (lp_integer_sgn(lp_Z, &v->value.z) < 0) {
      Integer tmp;
      lp_integer_abs(lp_Z, tmp.get(), &v->value.z);
      os << "(- " << tmp << ")";
    } else {
      os << lp_integer_to_string(&v->value.z);
    }
  } else if (v->type == LP_VALUE_RATIONAL) {
    Integer n, d;
    lp_rational_get_num(&v->value.q, n.get());
    lp_rational_get_den(&v->value.q, d.get());
    lp_integer_abs(lp_Z, n.get(), n.get());
    if (lp_rational_sgn(&v->value.q) < 0) {
      os << "(- (/ " << lp_integer_to_string(n.get()) << " " << lp_integer_to_string(d.get()) << "))";
    } else {
      os << "(/ " << lp_integer_to_string(n.get()) << " " << lp_integer_to_string(d.get()) << ")";
    }
  } else if (v->type == LP_VALUE_DYADIC_RATIONAL) {
    Integer n, d;
    lp_dyadic_rational_get_num(&v->value.dy_q, n.get());
    lp_dyadic_rational_get_den(&v->value.dy_q, d.get());
    lp_integer_abs(lp_Z, n.get(), n.get());
    if (lp_dyadic_rational_sgn(&v->value.dy_q) < 0) {
      os << "(- (/ " << lp_integer_to_string(n.get()) << " " << lp_integer_to_string(d.get()) << "))";
    } else {
      os << "(/ " << lp_integer_to_string(n.get()) << " " << lp_integer_to_string(d.get()) << ")";
    }
  } else if (v->type == LP_VALUE_ALGEBRAIC) {
    for (size_t i = 0; i < 10; ++i) {
      lp_algebraic_number_refine_const(&v->value.a);
    }
    if (v->value.a.I.is_point) {
      Value value;
      lp_value_construct(value.get(), LP_VALUE_DYADIC_RATIONAL, &v->value.a.I.a);
      render(os, value);
    } else {
      if (approx_from_below) {
        Value value;
        lp_value_construct(value.get(), LP_VALUE_DYADIC_RATIONAL, &v->value.a.I.a);
        render(os, value);
      } else {
        Value value;
        lp_value_construct(value.get(), LP_VALUE_DYADIC_RATIONAL, &v->value.a.I.b);
        render(os, value);
      }
    }
  } else {
    std::cout << "Skipping " << val << std::endl;
  }
}

void render(std::ostream& os, const Variable& v, const Interval& interval) {
  const lp_interval_t* i = interval.get();
  if (i->is_point) {
    os << "(assert (= " << v << " ";
    render(os, Value(i->a));
    os << "))" << std::endl;
  } else {
    if (i->a.type != LP_VALUE_MINUS_INFINITY) {
      os << "(assert (< ";
      render(os, Value(i->a), false);
      os << " " << v << "))" << std::endl;
    }
    if (i->b.type != LP_VALUE_PLUS_INFINITY) {
      os << "(assert (< " << v << " ";
      render(os, Value(i->b), true);
      os << "))" << std::endl;
    }
  }
}

void CDCACDebugger::check_interval(const Assignment& a, const Variable& variable, const CACInterval& i) {
  ++mCheckCounter;

  std::cout << "Writing interval to cac-debug-" + std::to_string(mCheckCounter) + ".smt2" << std::endl;

  std::ofstream out("cac-debug-" + std::to_string(mCheckCounter) + ".smt2");
  out << "(set-logic QF_NRA)" << std::endl;
  for (const auto& v: mVariables) {
    out << "(declare-fun " << v << " () Real)" << std::endl;
  }
  out << "; Constraints used as origins" << std::endl;
  for (const auto& o: i.mOrigins) {
    out << "(assert " << o << ")" << std::endl;
  }
  out << "; Current assignment" << std::endl;
  for (const auto& v: mVariables) {
    if (a.has(v)) {
      out << "(assert (= " << v << " ";
      render(out, a.retrieve(v));
      out << "))" << std::endl;
    }
  }
  out << "; Excluded interval for " << variable << std::endl;
  render(out, variable, i.mInterval);
  out << "(check-sat)" << std::endl;
}

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4