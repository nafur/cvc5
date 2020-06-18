#include "constraints.h"

#include "../poly_conversion.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

using namespace poly;

void Constraints::add_constraint(const Polynomial& lhs,
                                 SignCondition sc,
                                 Node n)
{
  mConstraints.emplace_back(lhs, sc, n);
}

void Constraints::add_constraint(Node n)
{
  auto c = as_poly_constraint(n, mVarMapper);
  add_constraint(c.first, c.second, n);
}

const Constraints::ConstraintVector& Constraints::get_constraints() const
{
  return mConstraints;
}

void Constraints::reset() { mConstraints.clear(); }

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
