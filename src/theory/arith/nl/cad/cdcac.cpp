#include "cdcac.h"

#include "projections.h"

namespace CVC4 {

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    container_to_stream(os, v);
    return os;
}

namespace theory {
namespace arith {
namespace nl {
namespace cad {

template <typename T>
void remove_duplicates(std::vector<T>& v)
{
  std::sort(v.begin(), v.end());
  v.erase(std::unique(v.begin(), v.end()), v.end());
}

CDCAC::CDCAC() : debugger(mVariableOrdering) {}

CDCAC::CDCAC(const std::vector<Variable>& ordering)
    : mVariableOrdering(ordering), debugger(mVariableOrdering)
{
}

void CDCAC::reset()
{
  mConstraints.reset();
  mAssignment.clear();
}

void CDCAC::compute_variable_ordering()
{
  VariableCollector vc;
  for (const auto& c : mConstraints.get_constraints())
  {
    vc(std::get<0>(c));
  }
  mVariableOrdering = vc.get_variables();
  // TODO(Gereon): Figure out how to set custom variable ordering in poly
  std::sort(mVariableOrdering.begin(),
            mVariableOrdering.end(),
            [](const Variable& a, const Variable& b) {
              return lp_variable_order_cmp(
                         poly::Context::get_context().get_variable_order(), a.get_internal(), b.get_internal())
                     < 0;
            });
  Trace("cdcac") << "Variable ordering is now " << mVariableOrdering
                 << std::endl;
}

Constraints& CDCAC::get_constraints() { return mConstraints; }
const Constraints& CDCAC::get_constraints() const { return mConstraints; }

const Assignment& CDCAC::get_model() const { return mAssignment; }

const std::vector<Variable>& CDCAC::get_variable_ordering() const
{
  return mVariableOrdering;
}

std::vector<CACInterval> CDCAC::get_unsat_intervals(
    std::size_t cur_variable) const
{
  std::vector<CACInterval> res;
  for (const auto& c : mConstraints.get_constraints())
  {
    const Polynomial& p = std::get<0>(c);
    SignCondition sc = std::get<1>(c);
    const Node& n = std::get<2>(c);

    if (main_variable(p) != mVariableOrdering[cur_variable])
    {
      Trace("cdcac") << "Skipping " << p << " as it is not univariate."
                     << std::endl;
      continue;
    }

    Trace("cdcac") << "Infeasible intervals for " << p << " " << sc
                   << " 0 over " << mAssignment << std::endl;
    auto intervals = infeasible_regions(p, mAssignment, sc);
    for (const auto& i : intervals)
    {
      Trace("cdcac") << "-> " << i << std::endl;
      std::vector<Polynomial> l, u, m, d;
      // TODO(Gereon): Factorize polynomials here.
      if (!is_infinity(get_lower(i))) l.emplace_back(p);
      if (!is_infinity(get_upper(i))) u.emplace_back(p);
      m.emplace_back(p);
      res.emplace_back(CACInterval{i, l, u, m, d, {n}});
    }
  }
  clean_intervals(res);
  return res;
}

std::vector<Polynomial> CDCAC::required_coefficients(const Polynomial& p) const
{
  std::vector<Polynomial> res;
  for (long deg = degree(p); deg >= 0; --deg)
  {
    auto coeff = coefficient(p, deg);
    if (lp_polynomial_is_constant(coeff.get_internal())) break;
    res.emplace_back(coeff);
    if (evaluate_polynomial_constraint(coeff, mAssignment, SignCondition::NE))
    {
      break;
    }
  }
  return res;
}

void add_polynomial(
    std::vector<std::pair<Polynomial, std::vector<Node>>>& polys,
    const Polynomial& poly,
    const std::vector<Node>& origin)
{
  for (const auto& p : square_free_factors(poly))
  {
    if (is_constant(p)) continue;
    polys.emplace_back(p, origin);
  }
}

std::vector<Polynomial> CDCAC::construct_characterization(
    const std::vector<CACInterval>& intervals)
{
  // TODO(Gereon): origins from a single interval are a squarefree basis. What
  // about resultants of polys from different intervals?
  Assert(!intervals.empty()) << "A covering can not be empty";
  // TODO(Gereon): We might want to reduce the covering by removing redundancies
  // as of section 4.5.2
  Trace("cdcac") << "Constructing characterization now" << std::endl;
  std::vector<Polynomial> res;

  for (const auto& i : intervals)
  {
    Trace("cdcac") << "Considering " << i.mInterval << std::endl;
    Trace("cdcac") << "-> " << i.mLowerPolys << " / " << i.mUpperPolys
                   << " and " << i.mMainPolys << " / " << i.mDownPolys
                   << std::endl;
    Trace("cdcac") << "-> " << i.mOrigins << std::endl;
    for (const auto& p : i.mDownPolys)
    {
      add_polynomial(res, p);
    }
    for (const auto& p : i.mMainPolys)
    {
      Trace("cdcac") << "Discriminant of " << p << " -> " << discriminant(p)
                     << std::endl;
      add_polynomial(res, discriminant(p));

      for (const auto& q : required_coefficients(p))
      {
        Trace("cdcac") << "Coeff of " << p << " -> " << q << std::endl;
        add_polynomial(res, q);
      }
      // TODO(Gereon): Only add if p(s \times a) = a for some a <= l
      for (const auto& q : i.mLowerPolys)
      {
        if (p == q) continue;
        Trace("cdcac") << "Resultant of " << p << " and " << q << " -> "
                       << resultant(p, q) << std::endl;
        add_polynomial(res, resultant(p, q));
      }
      // TODO(Gereon): Only add if p(s \times a) = a for some a >= u
      for (const auto& q : i.mUpperPolys)
      {
        if (p == q) continue;
        Trace("cdcac") << "Resultant of " << p << " and " << q << " -> "
                       << resultant(p, q) << std::endl;
        add_polynomial(res, resultant(p, q));
      }
    }
  }

  for (std::size_t i = 0; i < intervals.size() - 1; ++i)
  {
    for (const auto& p : intervals[i].mUpperPolys)
    {
      for (const auto& q : intervals[i + 1].mLowerPolys)
      {
        Trace("cdcac") << "Resultant of " << p << " and " << q << " -> "
                       << resultant(p, q) << std::endl;
        add_polynomial(res, resultant(p, q));
      }
    }
  }

  remove_duplicates(res);
  make_finest_square_free_basis(res);

  return res;
}

CACInterval CDCAC::interval_from_characterization(
    const std::vector<Polynomial>& characterization,
    std::size_t cur_variable,
    const Value& sample)
{
  std::vector<Polynomial> l;
  std::vector<Polynomial> u;
  std::vector<Polynomial> m;
  std::vector<Polynomial> d;

  for (const auto& p : characterization)
  {
    if (main_variable(p) == mVariableOrdering[cur_variable])
    {
      m.emplace_back(p);
    }
    else
    {
      d.emplace_back(p);
    }
  }

  std::vector<Value> roots;
  roots.emplace_back(Value::minus_infty());
  for (const auto& p : m)
  {
    auto tmp = isolate_real_roots(p, mAssignment);
    roots.insert(roots.end(), tmp.begin(), tmp.end());
  }
  roots.emplace_back(Value::plus_infty());
  std::sort(roots.begin(), roots.end());

  Value lower;
  Value upper;
  for (std::size_t i = 0; i < roots.size(); ++i)
  {
    if (sample < roots[i])
    {
      lower = roots[i - 1];
      upper = roots[i];
      break;
    }
    if (roots[i] == sample)
    {
      lower = sample;
      upper = sample;
      break;
    }
  }
  Assert(lower != Value() && upper != Value());

  if (lower != Value::minus_infty())
  {
    mAssignment.set(mVariableOrdering[cur_variable], lower);
    for (const auto& p : m)
    {
      if (evaluate_polynomial_constraint(p, mAssignment, SignCondition::EQ))
      {
        l.emplace_back(p);
      }
    }
    mAssignment.unset(mVariableOrdering[cur_variable]);
  }
  if (upper != Value::plus_infty())
  {
    mAssignment.set(mVariableOrdering[cur_variable], upper);
    for (const auto& p : m)
    {
      if (evaluate_polynomial_constraint(p, mAssignment, SignCondition::EQ))
      {
        u.emplace_back(p);
      }
    }
    mAssignment.unset(mVariableOrdering[cur_variable]);
  }

  return CACInterval{Interval(lower, upper), l, u, m, d, {}};
}

std::vector<CACInterval> CDCAC::get_unsat_cover(std::size_t cur_variable)
{
  if (cur_variable == 0)
  {
    Trace("cdcac") << "******************** CDCAC Check" << std::endl;
    for (const auto& c : mConstraints.get_constraints())
    {
      Trace("cdcac") << "-> " << std::get<0>(c) << " " << std::get<1>(c)
                     << " 0 from " << std::get<2>(c) << std::endl;
    }
  }
  Trace("cdcac") << "Looking for unsat cover for "
                 << mVariableOrdering[cur_variable] << " from "
                 << mVariableOrdering << std::endl;
  std::vector<CACInterval> intervals = get_unsat_intervals(cur_variable);
  Trace("cdcac") << "Unsat intervals for " << mVariableOrdering[cur_variable]
                 << ":" << std::endl;
  for (const auto& i : intervals)
    Trace("cdcac") << "-> " << i.mInterval << " from " << i.mOrigins
                   << std::endl;
  Value sample;

  while (sample_outside(intervals, sample))
  {
    mAssignment.set(mVariableOrdering[cur_variable], sample);
    Trace("cdcac") << "Sample: " << mAssignment << std::endl;
    if (cur_variable == mVariableOrdering.size() - 1)
    {
      // We have a full assignment. SAT!
      Trace("cdcac") << "Found full assignment: " << mAssignment << std::endl;
      return {};
    }
    auto cov = get_unsat_cover(cur_variable + 1);
    if (cov.empty())
    {
      // Found SAT!
      Trace("cdcac") << "SAT!" << std::endl;
      return {};
    }
    Trace("cdcac") << "Refuting Sample: " << mAssignment << std::endl;
    auto characterization = construct_characterization(cov);
    Trace("cdcac") << "Characterization: " << characterization << std::endl;

    mAssignment.unset(mVariableOrdering[cur_variable]);

    auto new_interval =
        interval_from_characterization(characterization, cur_variable, sample);
    new_interval.mOrigins = collect_constraints(cov);
    Trace("cdcac") << "Collected origins: " << new_interval.mOrigins
                   << std::endl;

    intervals.emplace_back(new_interval);
    Trace("cdcac") << "Added " << intervals.back().mInterval << std::endl;
    // debugger.check_interval(mAssignment, mVariableOrdering[cur_variable],
    // intervals.back());
    clean_intervals(intervals);
    Trace("cdcac") << "Now we have for " << mVariableOrdering[cur_variable]
                   << ":" << std::endl;
    for (const auto& i : intervals)
      Trace("cdcac") << "-> " << i.mInterval << " (from " << i.mOrigins << ")"
                     << std::endl;
  }

  Trace("cdcac") << "Returning intervals for "
                 << mVariableOrdering[cur_variable] << ":" << std::endl;
  for (const auto& i : intervals)
    Trace("cdcac") << "-> " << i.mInterval << std::endl;
  return intervals;
}

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
