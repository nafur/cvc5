/*********************                                                        */
/*! \file inference_manager.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Gereon Kremer
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2020 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Customized inference manager for the theory of nonlinear arithmetic
 **/

#include "cvc4_private.h"

#ifndef CVC4__THEORY__ARITH__INFERENCE_MANAGER_H
#define CVC4__THEORY__ARITH__INFERENCE_MANAGER_H

#include <map>
#include <vector>

#include "theory/arith/arith_lemma.h"
#include "theory/arith/arith_state.h"
#include "theory/arith/nl/inference.h"
#include "theory/arith/nl/nl_lemma_utils.h"
#include "theory/inference_manager_buffered.h"

namespace CVC4 {
namespace theory {
namespace arith {

class TheoryArith;

class InferenceManager : public InferenceManagerBuffered
{
 public:
  InferenceManager(TheoryArith& ta, ArithState& astate, ProofNodeManager* pnm);

  void addLemma(std::shared_ptr<ArithLemma> lemma);
  void addLemma(const ArithLemma& lemma);
  void addLemma(const Node& lemma, nl::Inference inftype);

  void addWaitingLemma(std::shared_ptr<ArithLemma> lemma);
  void addWaitingLemma(const ArithLemma& lemma);
  void addWaitingLemma(const Node& lemma, nl::Inference inftype);

  void flushWaitingLemmas();

  void addConflict(const Node& conf, nl::Inference inftype);

 private:
  std::vector<std::shared_ptr<ArithLemma>> d_waitingLem;
};

}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
