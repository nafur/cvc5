#include "preprocessing/passes/arith_split_zero_equalities.h"

#include "base/check.h"
#include "expr/node_traversal.h"

namespace CVC4 {
namespace preprocessing {
namespace passes {

using namespace CVC4::theory;

ArithSplitZeroEqualities::ArithSplitZeroEqualities(
    PreprocessingPassContext* preprocContext)
    : PreprocessingPass(preprocContext, "arith-split-zero-equalities"){};

namespace {
bool is_zero_equality(const TNode& node)
{
  if (node.getKind() != Kind::EQUAL) return false;
  return std::any_of(node.begin(), node.end(), [](const TNode& n) {
    return n.isConst() && n.getConst<Rational>().isZero();
  });
}

Node build_splitting_lemma(const TNode& node)
{
  Assert(node.getKind() == Kind::NONLINEAR_MULT);
  auto* nm = NodeManager::currentNM();
  std::vector<Node> children;
  for (const auto& c : node)
  {
    children.emplace_back(nm->mkNode(Kind::EQUAL, c, nm->mkConst(Rational())));
  }
  return Rewriter::rewrite(
      nm->mkNode(Kind::IMPLIES, node, nm->mkNode(Kind::OR, children)));
}
}  // namespace

PreprocessingPassResult ArithSplitZeroEqualities::applyInternal(
    AssertionPipeline* assertionsToPreprocess)
{
  std::size_t cursize = assertionsToPreprocess->size();
  for (std::size_t i = 0; i < cursize; ++i)
  {
    const auto& a = (*assertionsToPreprocess)[i];
    for (const auto& node : NodeDfsIterable(a))
    {
      if (!is_zero_equality(node)) continue;

      for (const auto& c : node)
      {
        if (c.getKind() == Kind::NONLINEAR_MULT)
        {
          Node lemma = build_splitting_lemma(c);
          Trace("arith-split-zero-equalities") << node << " -> " << lemma << std::endl;
          assertionsToPreprocess->push_back(lemma);
        }
      }
    }
  }

  return PreprocessingPassResult::NO_CONFLICT;
}

}  // namespace passes
}  // namespace preprocessing
}  // namespace CVC4
