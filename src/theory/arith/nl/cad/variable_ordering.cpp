#include "variable_ordering.h"

#include "cdcac_stats.h"

#include "util/poly_util.h"

#ifdef CVC4_USE_DLIB
#include <dlib/svm.h>
#endif

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

using namespace poly;

std::vector<poly_utils::VariableInformation> collect_information(
    const Constraints::ConstraintVector& polys, bool with_totals)
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
      poly_utils::getVariableInformation(res.back(), std::get<0>(c));
    }
  }
  if (with_totals)
  {
    res.emplace_back();
    for (const auto& c : polys)
    {
      poly_utils::getVariableInformation(res.back(), std::get<0>(c));
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

std::vector<poly::Variable> sort_byid(const Constraints::ConstraintVector& polys)
{
  auto vi = collect_information(polys);
  std::sort(
      vi.begin(),
      vi.end(),
      [](const poly_utils::VariableInformation& a,
         const poly_utils::VariableInformation& b) { return a.var < b.var; });
  return get_variables(vi);
};

std::vector<poly::Variable> sort_brown(const Constraints::ConstraintVector& polys)
{
  auto vi = collect_information(polys);
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
  return get_variables(vi);
};

std::vector<poly::Variable> sort_triangular(const Constraints::ConstraintVector& polys)
{
  auto vi = collect_information(polys);
  std::sort(vi.begin(),
            vi.end(),
            [](const poly_utils::VariableInformation& a,
               const poly_utils::VariableInformation& b) {
              if (a.max_degree != b.max_degree)
                return a.max_degree > b.max_degree;
              if (a.max_lc_degree != b.max_lc_degree)
                return a.max_lc_degree > b.max_lc_degree;
              return a.sum_poly_degree > b.sum_poly_degree;
            });
  return get_variables(vi);
};

#ifdef CVC4_USE_DLIB
class VOMLState
{
    using sample_type = dlib::matrix<double, NRAFeatures::feature_count, 1>;
    using kernel_type = dlib::radial_basis_kernel<sample_type>;
    using tester_type = dlib::decision_function<kernel_type>;
    std::vector<tester_type> regressions;
public:
  VOMLState(const std::string& filename) {
    dlib::deserialize(filename) >> regressions;
  }
  std::size_t operator()(const NRAFeatures& features) const {
      double min = std::numeric_limits<double>::max();
      std::size_t res = 0;
      auto f = dlib::mat(features.to_feature_vector());
      for (std::size_t i = 0; i < regressions.size(); ++i) {
          double cur = regressions[i](f);
          if (cur < min) {
              min = cur;
              res = i;
          }
      }
      return res;
  }
};

std::vector<poly::Variable> sort_ml(const Constraints::ConstraintVector& polys, std::unique_ptr<VOMLState>& state) {
  if (!state) {
    state.reset(new VOMLState("vo-ml.model"));
  }
  std::size_t selection = (*state)(cad::NRAFeatures(polys));
  switch (selection) {
    case 0: return sort_brown(polys);
    case 1: return sort_byid(polys);
    case 2: return sort_triangular(polys);
    default:
      Notice() << "Learned heuristic selected unsupported variable ordering: " << selection;
      return sort_brown(polys);
  }
}
#else
class VOMLState {};
#endif

VariableOrdering::VariableOrdering() : state_ml(nullptr) {}
VariableOrdering::~VariableOrdering() {}

std::vector<poly::Variable> VariableOrdering::operator()(
    const Constraints::ConstraintVector& polys,
    VariableOrderingStrategy vos) const
{
  switch (vos)
  {
    case VariableOrderingStrategy::ByID: return sort_byid(polys);
    case VariableOrderingStrategy::Brown: return sort_brown(polys);
    case VariableOrderingStrategy::Triangular: return sort_triangular(polys);
#ifdef CVC4_USE_DLIB
    case VariableOrderingStrategy::ML: return sort_ml(polys, state_ml);
#endif
    default: Assert(false) << "Unsupported variable ordering.";
  }
  return {};
}

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4