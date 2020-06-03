#include "cdcac.h"

#include "../libpoly/ran.h"

#include "projections.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

inline bool compare_for_cleanup(const Interval& lhs, const Interval& rhs) {
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

void clean_intervals(std::vector<CACInterval>& intervals) {
    // Simplifies removal of redundancies later on.
    if (intervals.size() < 2) return;

    // Sort intervals.    
    std::sort(intervals.begin(), intervals.end(),
        [](const CACInterval& lhs, const CACInterval& rhs) {
            return compare_for_cleanup(lhs.mInterval, rhs.mInterval);
        }
    );
    //Trace("cad-check") << "Ordered intervals:" << std::endl;
    //for (const auto& i: intervals) Trace("cad-check") << "-> " << i.mInterval << std::endl;

    // Remove intervals that are covered by others.
    // Implementation roughly follows https://en.cppreference.com/w/cpp/algorithm/remove
    // Find first interval that covers the next one.
    std::size_t first = 0;
    for (; first < intervals.size() - 1; ++first) {
        if (interval_covers(intervals[first].mInterval, intervals[first + 1].mInterval)) {
            //Trace("cad-check") << first << " covers " << (first+1) << std::endl;
            break;
        }
    }
    // If such an interval exists, remove accordingly.
    if (first < intervals.size() - 1) {
        for (std::size_t i = first + 2; i < intervals.size(); ++i) {
            if (!interval_covers(intervals[first].mInterval, intervals[i].mInterval)) {
                // Interval is not covered. Move it to the front and bump front.
                //Trace("cad-check") << first << " does not cover " << i << std::endl;
                ++first;
                intervals[first] = std::move(intervals[i]);
            }
            // Else: Interval is covered as well.
            else {
                //Trace("cad-check") << first << " also covers " << i << std::endl;
            }
        }
        // Erase trailing values
        while (intervals.size() > first + 1) {
            intervals.pop_back();
        }
    }
}

std::vector<CACInterval> CDCAC::get_unsat_intervals(std::size_t cur_variable) const {
    std::vector<CACInterval> res;
    for (const auto& c: mConstraints.get_constraints()) {
        const Polynomial& p = c.first;
        SignCondition sc = c.second;

        if (main_variable(p) != mVariableOrdering[cur_variable]) {
            Trace("cad-check") << "Skipping " << p << " as it is not univariate." << std::endl;
            continue;
        }

        auto intervals = infeasible_regions(p, mAssignment, sc);
        for (const auto& i: intervals) {
            std::vector<Polynomial> l, u, m, d;
            // TODO(Gereon): Factorize polynomials here.
            if (!lower_is_infty(i)) l.emplace_back(p);
            if (!upper_is_infty(i)) u.emplace_back(p);
            m.emplace_back(p);
            res.emplace_back(CACInterval{i, l, u, m, d});
        }
    }
    clean_intervals(res);
    return res;
}

bool CDCAC::sample_outside(const std::vector<CACInterval>& infeasible, Value& sample) const {
    if (infeasible.empty()) {
        sample = Integer(0);
        return true;
    }
    if (!lower_is_infty(infeasible.front().mInterval)) {
        const auto* i = infeasible.front().mInterval.get();
        sample = sample_between(Value::minus_infty().get(), true, &i->a, !i->a_open);
        return true;
    }
    for (std::size_t i = 0; i < infeasible.size() - 1; ++i) {
        if (!interval_connect(infeasible[i].mInterval, infeasible[i+1].mInterval)) {
            const auto* l = infeasible[i].mInterval.get();
            const auto* r = infeasible[i+1].mInterval.get();

            if (l->is_point) {
                sample = sample_between(&l->a, true, &r->a, !r->a_open);
            } else {
                sample = sample_between(&l->b, !l->b_open, &r->a, !r->a_open);
            }
            return true;
        }
    }
    if (!upper_is_infty(infeasible.back().mInterval)) {
        const auto* i = infeasible.back().mInterval.get();
        if (i->is_point) {
            sample = sample_between(&i->a, true, Value::plus_infty().get(), true);
        } else {
            sample = sample_between(&i->b, !i->b_open, Value::plus_infty().get(), true);
        }
        return true;
    }
    return false;
}

std::vector<Polynomial> CDCAC::required_coefficients(const Polynomial& p) const {
    std::vector<Polynomial> res;
    for (long deg = degree(p); deg >= 0; --deg)
    {
        auto coeff = coefficient(p, deg);
        if (lp_polynomial_is_constant(coeff.get())) break;
        res.emplace_back(coeff);
        if (evaluate_polynomial_constraint(coeff, mAssignment, SignCondition::NE)) {
            break;
        }
    }
    return res;
}

std::vector<Polynomial> CDCAC::construct_characterization(const std::vector<CACInterval>& intervals) {
    Assert(!intervals.empty()) << "A covering can not be empty";
    // TODO(Gereon): We might want to reduce the covering by removing redundancies as of section 4.5.2
    Trace("cad-check") << "Constructing characterization now" << std::endl;
    std::vector<Polynomial> res;

    for (const auto& i: intervals) {
        add_polynomials(res, i.mDownPolys);
        for (const auto& p: i.mMainPolys) {
            add_polynomial(res, discriminant(p));
            // TODO(Gereon): Only add required coefficients
            add_polynomials(res, required_coefficients(p));
            // TODO(Gereon): Only add if p(s \times a) = a for some a <= l
            for (const auto& q: i.mLowerPolys) {
                add_polynomial(res, resultant(p, q));
            }
            // TODO(Gereon): Only add if p(s \times a) = a for some a >= u
            for (const auto& q: i.mUpperPolys) {
                add_polynomial(res, resultant(p, q));
            }
        }
    }

    for (std::size_t i = 0; i < intervals.size() - 1; ++i) {
        for (const auto& p: intervals[i].mUpperPolys) {
            for (const auto& q: intervals[i+1].mLowerPolys) {
                add_polynomial(res, resultant(p, q));
            }
        }
    }

    reduce_projection_polynomials(res);

    return res;
}

CACInterval CDCAC::interval_from_characterization(const std::vector<Polynomial>& characterization, std::size_t cur_variable, const Value& sample) {
    std::vector<Polynomial> l;
    std::vector<Polynomial> u;
    std::vector<Polynomial> m;
    std::vector<Polynomial> d;

    for (const auto& p: characterization) {
        if (main_variable(p) == mVariableOrdering[cur_variable]) {
            m.emplace_back(p);
        } else {
            d.emplace_back(p);
        }
    }

    std::vector<Value> roots;
    roots.emplace_back(Value::minus_infty());
    for (const auto& p: m) {
        auto tmp = isolate_real_roots(p, mAssignment);
        roots.insert(roots.end(), tmp.begin(), tmp.end());
    }
    roots.emplace_back(Value::plus_infty());
    std::sort(roots.begin(), roots.end());

    Trace("cad-check") << "Roots: " << roots << std::endl;

    Value lower;
    Value upper;
    for (std::size_t i = 0; i < roots.size(); ++i) {
        if (sample < roots[i]) {
            lower = roots[i-1];
            upper = roots[i];
            break;
        }
        if (roots[i] == sample) {
            lower = sample;
            upper = sample;
            break;
        }
    }
    Assert(lower != Value() && upper != Value());

    if (lower != Value::minus_infty()) {
        mAssignment.set(mVariableOrdering[cur_variable], lower);
        for (const auto& p: m) {
            if (evaluate_polynomial_constraint(p, mAssignment, SignCondition::EQ)) {
                l.emplace_back(p);
            }
        }
        mAssignment.unset(mVariableOrdering[cur_variable]);
    }
    if (upper != Value::plus_infty()) {
        mAssignment.set(mVariableOrdering[cur_variable], upper);
        for (const auto& p: m) {
            if (evaluate_polynomial_constraint(p, mAssignment, SignCondition::EQ)) {
                u.emplace_back(p);
            }
        }
        mAssignment.unset(mVariableOrdering[cur_variable]);
    }

    return CACInterval{
        Interval(lower, upper), l, u, m, d
    };
}

std::vector<CACInterval> CDCAC::get_unsat_cover(std::size_t cur_variable) {
    std::vector<CACInterval> intervals = get_unsat_intervals(cur_variable);
    Trace("cad-check") << "Unsat intervals for " << mVariableOrdering[cur_variable] << ":" << std::endl;
    for (const auto& i: intervals) Trace("cad-check") << "-> " << i.mInterval << std::endl;
    Value sample;

    std::size_t iterations = 0;

    while (sample_outside(intervals, sample)) {
        Trace("cad-check") << "Sample " << mVariableOrdering[cur_variable] << " = " << sample << std::endl;
        mAssignment.set(mVariableOrdering[cur_variable], sample);
        Trace("cad-check") << "Now: " << mAssignment << std::endl;
        if (cur_variable == mVariableOrdering.size() - 1) {
            // We have a full assignment. SAT!
            Trace("cad-check") << "Found full assignment: " << mAssignment << std::endl;
            return {};
        }
        auto cov = get_unsat_cover(cur_variable + 1);
        if (cov.empty()) {
            // Found SAT!
            Trace("cad-check") << "SAT!" << std::endl;
            return {};
        }
        auto characterization = construct_characterization(cov);
        Trace("cad-check") << "Characterization: " << characterization << std::endl;

        mAssignment.unset(mVariableOrdering[cur_variable]);
        
        auto new_interval = interval_from_characterization(characterization, cur_variable, sample);
        intervals.emplace_back(new_interval);
        clean_intervals(intervals);
        Trace("cad-check") << "Now we have for " << mVariableOrdering[cur_variable] << ":" << std::endl;
        for (const auto& i: intervals) Trace("cad-check") << "-> " << i.mInterval << std::endl;
        
        if (iterations > 2) break;
        ++iterations;
    }

    Trace("cad-check") << "Returning intervals for " << mVariableOrdering[cur_variable] << ":" << std::endl;
    for (const auto& i: intervals) Trace("cad-check") << "-> " << i.mInterval << std::endl;
    return intervals;
}

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
