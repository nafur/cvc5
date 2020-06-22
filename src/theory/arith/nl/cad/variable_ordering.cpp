#include "variable_ordering.h"

#include "util/poly_util.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

using namespace poly;

std::vector<poly_utils::VariableInformation> collect(
    const Constraints::ConstraintVector& polys)
{
  VariableCollector vc;
  for (const auto& c : polys)
  {
    vc(std::get<0>(c));
  }
  std::vector<poly_utils::VariableInformation> res;
  for (const auto& v : vc.get_variables())
  {
    res.emplace_back();
    res.back().var = v;
    for (const auto& c : polys)
    {
      poly_utils::get_variable_information(res.back(), std::get<0>(c));
    }
  }
  return res;
}

std::vector<poly::Variable> get_variables(
    const std::vector<poly_utils::VariableInformation>& vi)
{
  std::vector<poly::Variable> res;
  for (const auto& v : vi)
  {
    res.emplace_back(v.var);
  }
  return res;
}

void sort_byid(std::vector<poly_utils::VariableInformation>& vi)
{
  std::sort(
      vi.begin(),
      vi.end(),
      [](const poly_utils::VariableInformation& a,
         const poly_utils::VariableInformation& b) { return a.var < b.var; });
};

void sort_brown(std::vector<poly_utils::VariableInformation>& vi)
{
  std::sort(vi.begin(),
            vi.end(),
            [](const poly_utils::VariableInformation& a,
               const poly_utils::VariableInformation& b) {
              if (a.max_degree != b.max_degree)
                return a.max_degree > b.max_degree;
              if (a.max_terms_tdegree != b.max_terms_tdegree)
                return a.max_terms_tdegree > b.max_terms_tdegree;
              return a.num_terms > b.num_terms;
            });
};

void sort_triangular(std::vector<poly_utils::VariableInformation>& vi)
{
  std::sort(vi.begin(),
            vi.end(),
            [](const poly_utils::VariableInformation& a,
               const poly_utils::VariableInformation& b) {
              if (a.max_degree != b.max_degree)
                return a.max_degree > b.max_degree;
              if (a.max_lc_degree != b.max_lc_degree)
                return a.max_lc_degree > b.max_lc_degree;
              return a.sum_degree > b.sum_degree;
            });
};

std::vector<poly::Variable> variable_ordering(
    const Constraints::ConstraintVector& polys, VariableOrdering vo)
{
  std::vector<poly_utils::VariableInformation> vi = collect(polys);
  switch (vo)
  {
    case VariableOrdering::ByID: sort_byid(vi); break;
    case VariableOrdering::Brown: sort_brown(vi); break;
    case VariableOrdering::Triangular: sort_triangular(vi); break;
    default: Assert(false) << "Unsupported variable ordering.";
  }
  Trace("cdcac") << "Computed variable ordering:" << std::endl;
  for (const auto& v : vi)
  {
    Trace("cdcac") << "\t" << v.var << ":\t" << v.max_degree << "\t/ "
                   << v.max_lc_degree << "\t/ " << v.sum_degree << "\t/ "
                   << v.max_terms_tdegree << "\t/ " << v.num_terms << std::endl;
  }
  return get_variables(vi);
}

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4