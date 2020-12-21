/*********************                                                        */
/*! \file constraints.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Gereon Kremer
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2020 by the authors listed in the file AUTHORS
 ** in the top-level source directory and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Implements a container for CAD constraints.
 **
 ** Implements a container for CAD constraints.
 **/

#include "theory/arith/nl/cad/constraints.h"

#ifdef CVC4_POLY_IMP

#include <algorithm>

#include "theory/arith/nl/poly_conversion.h"
#include "util/poly_util.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

void Constraints::addConstraint(const Constraint& c)
{
  d_constraints.emplace_back(c);
}

void Constraints::finalize() {
  sortConstraints();
}

Constraints::Constraint Constraints::asConstraint(TNode n) {
  auto c = as_poly_constraint(n, d_varMapper);
  return Constraint{c.first, c.second, n};
}

const Constraints::ConstraintVector& Constraints::getConstraints() const
{
  return d_constraints;
}

void Constraints::reset() { d_constraints.clear(); }

void Constraints::sortConstraints()
{
  for (const auto& c: d_constraints) {
    Trace("nl-cad") << "Sorting " << std::get<2>(c) << " -> " << std::get<0>(c) << std::endl;
  }
  for (auto& c : d_constraints)
  {
    auto* p = std::get<0>(c).get_internal();
    // set external and actually trigger reordering
    lp_polynomial_set_external(p);
    lp_polynomial_top_variable(p);
  }
  using Tpl = std::tuple<poly::Polynomial, poly::SignCondition, Node>;
  std::sort(d_constraints.begin(),
            d_constraints.end(),
            [](const Tpl& at, const Tpl& bt) {
              Trace("nl-cad") << "Compare " << std::get<2>(at) << " vs " << std::get<2>(bt) << std::endl;
              // Check if a is smaller than b
              const poly::Polynomial& a = std::get<0>(at);
              const poly::Polynomial& b = std::get<0>(bt);
              bool ua = is_univariate(a);
              bool ub = is_univariate(b);
              if (ua != ub) return ua;
              std::size_t tda = poly_utils::totalDegree(a);
              std::size_t tdb = poly_utils::totalDegree(b);
              if (tda != tdb) return tda < tdb;
              return degree(a) < degree(b);
            });
  for (const auto& c: d_constraints) {
    Trace("nl-cad") << std::get<0>(c) << " -> " << poly::main_variable(std::get<0>(c)) << std::endl;
  }
}

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
