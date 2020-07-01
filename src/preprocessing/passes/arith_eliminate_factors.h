#include "cvc4_private.h"

#ifndef CVC4__PREPROCESSING__PASSES__ARITH_ELIMINATE_FACTORS_H
#define CVC4__PREPROCESSING__PASSES__ARITH_ELIMINATE_FACTORS_H

#include <unordered_map>
#include <vector>

#include "expr/node.h"
#include "preprocessing/preprocessing_pass.h"
#include "preprocessing/preprocessing_pass_context.h"

namespace CVC4 {
namespace preprocessing {
namespace passes {

using NodeMap = std::unordered_map<Node, Node, NodeHashFunction>;

class ArithEliminateFactors : public PreprocessingPass
{
 public:
  ArithEliminateFactors(PreprocessingPassContext* preprocContext);

 protected:
  PreprocessingPassResult applyInternal(
      AssertionPipeline* assertionsToPreprocess) override;

 private:
  bool collect_strict_variable(const TNode& n,
        std::vector<Node>& positive,
        std::vector<Node>& negative);
  Node eliminate_factors(TNode n,
        const std::vector<Node>& positive,
        const std::vector<Node>& negative);

};

}  // namespace passes
}  // namespace preprocessing
}  // namespace CVC4

#endif /* CVC4__PREPROCESSING__PASSES__NL_EXT_PURIFY_H */
