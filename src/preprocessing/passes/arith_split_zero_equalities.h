#include "cvc4_private.h"

#ifndef CVC4__PREPROCESSING__PASSES__ARITH_SPLIT_ZERO_EQUALITIES_H
#define CVC4__PREPROCESSING__PASSES__ARITH_SPLIT_ZERO_EQUALITIES_H

#include <unordered_map>
#include <vector>

#include "expr/node.h"
#include "preprocessing/preprocessing_pass.h"
#include "preprocessing/preprocessing_pass_context.h"

namespace CVC4 {
namespace preprocessing {
namespace passes {

using NodeMap = std::unordered_map<Node, Node, NodeHashFunction>;

class ArithSplitZeroEqualities : public PreprocessingPass
{
 public:
  ArithSplitZeroEqualities(PreprocessingPassContext* preprocContext);

 protected:
  PreprocessingPassResult applyInternal(
      AssertionPipeline* assertionsToPreprocess) override;

 private:

};

}  // namespace passes
}  // namespace preprocessing
}  // namespace CVC4

#endif /* CVC4__PREPROCESSING__PASSES__ARITH_SPLIT_ZERO_EQUALITIES_H */
