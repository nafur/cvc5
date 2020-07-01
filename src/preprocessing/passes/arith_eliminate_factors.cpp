#include "preprocessing/passes/arith_eliminate_factors.h"

#include "base/check.h"
#include "theory/arith/normal_form.h"

namespace CVC4 {
namespace preprocessing {
namespace passes {

using namespace CVC4::theory;

arith::Polynomial get_constraint_polynomial(const TNode& n)
{
  Assert(n.getNumChildren() == 2);
  return arith::Polynomial::parsePolynomial(n[0])
         - arith::Polynomial::parsePolynomial(n[1]);
}

Maybe<std::pair<Rational, Node>> get_single_variable(const arith::Polynomial& p)
{
  if (p.size() != 1) return Maybe<std::pair<Rational, Node>>();
  arith::VarList vl = p.getHead().getVarList();
  if (vl.size() != 1) return Maybe<std::pair<Rational, Node>>();
  return std::pair<Rational, Node>{p.getHead().getConstant().getValue(),
                                   vl.getHead().getNode()};
}

bool is_arith_constraint(const TNode& n)
{
  switch (n.getKind())
  {
    case Kind::EQUAL: return true;
    case Kind::NOT: return is_arith_constraint(n[0]);
    case Kind::LT: return true;
    case Kind::LEQ: return true;
    case Kind::GT: return true;
    case Kind::GEQ: return true;
    default: return false;
  }
}

Maybe<std::tuple<TNode, Kind, TNode>> decompose_constraint(const TNode& n)
{
  switch (n.getKind())
  {
    case Kind::NOT:
    {
      auto r = decompose_constraint(n[0]);
      if (!r) return Maybe<std::tuple<TNode, Kind, TNode>>();
      switch (std::get<1>(r.value()))
      {
        case Kind::EQUAL:  // != will not be a bound that allows for factor
                           // elimination
          return Maybe<std::tuple<TNode, Kind, TNode>>();
        case Kind::LT:
          return std::tuple<TNode, Kind, TNode>{
              std::get<0>(r.value()), Kind::GEQ, std::get<2>(r.value())};
        case Kind::LEQ:
          return std::tuple<TNode, Kind, TNode>{
              std::get<0>(r.value()), Kind::GT, std::get<2>(r.value())};
        case Kind::GT:
          return std::tuple<TNode, Kind, TNode>{
              std::get<0>(r.value()), Kind::LEQ, std::get<2>(r.value())};
        case Kind::GEQ:
          return std::tuple<TNode, Kind, TNode>{
              std::get<0>(r.value()), Kind::LT, std::get<2>(r.value())};
        default: return Maybe<std::tuple<TNode, Kind, TNode>>();
      }
    }
    case Kind::EQUAL:
    case Kind::LT:
    case Kind::LEQ:
    case Kind::GT:
    case Kind::GEQ:
      return std::tuple<TNode, Kind, TNode>{n[0], n.getKind(), n[1]};
    default: return Maybe<std::tuple<TNode, Kind, TNode>>();
  }
}

/** Normalizes a bound constraint.
 * The input is a bound constraint of the form coeff*var rel rhs.
 * It normalizes it to var rel' rhs' by computing rhs' = rhs / coeff and
 * adapting rel accordingly (in case coeff is negative).
 */
void normalize_bound_constraint(const Rational& coeff,
                                const Node& var,
                                Kind& rel,
                                Rational& rhs)
{
  rhs = rhs / coeff;
  if (coeff < 0)
  {
    switch (rel)
    {
      case Kind::EQUAL: break;
      case Kind::LT: rel = Kind::GT; break;
      case Kind::LEQ: rel = Kind::GEQ; break;
      case Kind::GT: rel = Kind::LT; break;
      case Kind::GEQ: rel = Kind::LEQ; break;
      default: Assert(false) << "Unexpected relation " << rel;
    }
  }
}

bool ArithEliminateFactors::collect_strict_variable(const TNode& n,
                                                    std::vector<Node>& positive,
                                                    std::vector<Node>& negative)
{
  auto dc = decompose_constraint(n);
  if (!dc) return false;
  Kind rel = std::get<1>(dc.value());
  arith::Polynomial l =
      arith::Polynomial::parsePolynomial(std::get<0>(dc.value()));
  arith::Polynomial r =
      arith::Polynomial::parsePolynomial(std::get<2>(dc.value()));
  arith::Polynomial lhs = l - r;
  arith::SumPair sp = arith::SumPair::mkSumPair(lhs);
  auto cv = get_single_variable(sp.getPolynomial());
  if (!cv) return false;
  Rational c = sp.getConstant().getValue();
  const Node& var = cv.value().second;
  normalize_bound_constraint(cv.value().first, var, rel, c);

  switch (rel)
  {
    case Kind::EQUAL:
    {
      Assert(false)
          << "This is a direct substitution, why do we even see this? " << n;
      return false;
    }
    case Kind::LT:
    {
      if (c <= 0)
      {
        Trace("elim-factors") << n << " -> " << var << " < 0" << std::endl;
        negative.emplace_back(var);
      }
      return true;
    }
    case Kind::LEQ:
    {
      if (c < 0)
      {
        Trace("elim-factors") << n << " -> " << var << " < 0" << std::endl;
        negative.emplace_back(var);
      }
      return true;
    }
    case Kind::GT:
    {
      if (c >= 0)
      {
        Trace("elim-factors") << n << " -> " << var << " > 0" << std::endl;
        positive.emplace_back(var);
      }
      return true;
    }
    case Kind::GEQ:
    {
      if (c > 0)
      {
        Trace("elim-factors") << n << " -> " << var << " > 0" << std::endl;
        positive.emplace_back(var);
      }
      return true;
    }
    default:
    {
      Assert(false) << "This should not pass decompose_constraint(), why do we "
                       "even see this? "
                    << n;
      return false;
    }
  }
}
ArithEliminateFactors::ArithEliminateFactors(
    PreprocessingPassContext* preprocContext)
    : PreprocessingPass(preprocContext, "arith-eliminate-factors"){};

PreprocessingPassResult ArithEliminateFactors::applyInternal(
    AssertionPipeline* assertionsToPreprocess)
{
  std::size_t size = assertionsToPreprocess->size();
  std::vector<Node> positive;
  std::vector<Node> negative;
  std::vector<bool> used_as_bound(size, false);
  bool found_simplification = true;

  while (found_simplification)
  {
    found_simplification = false;
    bool found_new_bound = false;

    // Check assertions for constraints that are bounds
    for (std::size_t i = 0; i < size; ++i)
    {
      // Already used as bound
      if (used_as_bound[i])
      {
        continue;
      }
      bool res = collect_strict_variable(
          (*assertionsToPreprocess)[i], positive, negative);
      if (res)
      {
        used_as_bound[i] = true;
        found_new_bound = true;
      }
    }

    // If no new bounds were found: terminate
    if (!found_new_bound)
    {
      Trace("elim-factors") << "Did not find new bounds." << std::endl;
      return PreprocessingPassResult::NO_CONFLICT;
    }

    // Try to simplify assertions using the detected bounds
    for (std::size_t i = 0; i < size; ++i)
    {
      // Already used as bound
      if (used_as_bound[i])
      {
        continue;
      }
      TNode cur = (*assertionsToPreprocess)[i];
      Node res = eliminate_factors(cur, positive, negative);
      if (res != cur)
      {
        Trace("elim-factors")
            << "Simplified " << cur << " to " << res << std::endl;
        //found_simplification = true;
        //assertionsToPreprocess->replace(i, Rewriter::rewrite(res));
      }
    }
  }
  return PreprocessingPassResult::NO_CONFLICT;
}

Node ArithEliminateFactors::eliminate_factors(TNode n,
                                              const std::vector<Node>& positive,
                                              const std::vector<Node>& negative)
{
  auto dc = decompose_constraint(n);
  if (!dc) return n;
  Kind rel = std::get<1>(dc.value());
  arith::Polynomial l =
      arith::Polynomial::parsePolynomial(std::get<0>(dc.value()));
  arith::Polynomial r =
      arith::Polynomial::parsePolynomial(std::get<2>(dc.value()));
  arith::Polynomial lhs = l - r;

  bool found_simplification = false;

  for (const auto& p : positive)
  {
    while (lhs.divisible_by(p))
    {
      Trace("elim-factors")
          << "Simplified " << n << " with " << p << std::endl;
        lhs = lhs.divide_by(p);
        found_simplification = true;
    }
  }

  if (found_simplification) {
      return NodeManager::currentNM()->mkNode(
          rel, lhs.getNode(), NodeManager::currentNM()->mkConst(Rational(0))
      );
  }
  return n;
}

}  // namespace passes
}  // namespace preprocessing
}  // namespace CVC4
