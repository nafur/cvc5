#include "polynomial.h"

#include "base/output.h"
#include "value.h"

#include <poly/feasibility_set.h>

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

std::vector<Polynomial> square_free_factors(const Polynomial& p)
{
  lp_polynomial_t** factors = nullptr;
  std::size_t* multiplicities = nullptr;
  std::size_t size = 0;
  lp_polynomial_factor_square_free(p.get(), &factors, &multiplicities, &size);

  std::vector<Polynomial> res;
  for (std::size_t i = 0; i < size; ++i)
  {
    res.emplace_back(factors[i]);
  }
  free(factors);
  free(multiplicities);

  return res;
}

std::vector<Interval> infeasible_regions(const Polynomial& p, const Assignment& a, SignCondition sc) {
    lp_feasibility_set_t* feasible = lp_polynomial_constraint_get_feasible_set(p.get(), to_sign_condition(sc), 0, a.get());

    std::vector<Interval> regions;

    Value last_value = Value::minus_infty();
    int last_open = 0;

    for (std::size_t i = 0; i < feasible->size; ++i) {
        const lp_interval_t& cur = feasible->intervals[i];
        Value lower(lp_value_new_copy(&cur.a));

        //Trace("cad-check") << "Feasible region: " << lp_interval_to_string(&cur) << std::endl;

        if (lower.get()->type == LP_VALUE_MINUS_INFINITY) {
            // Do nothing if we start at -infty.
        } else if (last_value < lower) {
            // There is an infeasible open interval
            regions.emplace_back(last_value, !last_open, lower, !cur.a_open);
        } else if (last_open && cur.a_open && last_value == lower) {
            // There is an infeasible point interval
            regions.emplace_back(last_value);
        }
        if (cur.is_point) {
            last_value = std::move(lower);
            last_open = true;
        } else {
            last_value = lp_value_new_copy(&cur.b);
            last_open = cur.b_open;
        }
    }

    if (last_value.get()->type != LP_VALUE_PLUS_INFINITY) {
        // Add missing interval to +infty
        regions.emplace_back(last_value, !last_open, Value::plus_infty(), true);
    }

    lp_feasibility_set_delete(feasible);

    return regions;
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
