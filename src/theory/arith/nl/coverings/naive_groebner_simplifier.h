/******************************************************************************
 * Top contributors (to current version):
 *   Gereon Kremer
 *
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2021 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * Implements the CDCAC approach as described in
 * https://arxiv.org/pdf/2003.05633.pdf.
 */

#include "cvc5_private.h"

#ifndef CVC5__THEORY__ARITH__NL__COVERINGS__NAIVE_GROEBNER_SIMPLIFIER_H
#define CVC5__THEORY__ARITH__NL__COVERINGS__NAIVE_GROEBNER_SIMPLIFIER_H

#ifdef CVC5_POLY_IMP

#include <poly/polyxx.h>

#include <memory>
#include <optional>
#include <unordered_map>

#include "expr/node.h"
#include "smt/env_obj.h"

namespace cvc5::theory::arith::nl::coverings {

class NaiveGroebnerSimplifier: protected EnvObj
{
 public:
  struct NGSState;

  /** Initialize and simplify the inputs */
  NaiveGroebnerSimplifier(Env& env, const std::vector<Node>& inputs);
  ~NaiveGroebnerSimplifier();
 
  /** Return the simplified list of assertions */
  const std::vector<Node>& getSimplified() const { return d_simplified; }

  /** Check whether a direct conflict has been found */
  bool hasConflict() const { return d_conflict.has_value(); }
  /**
   * Return the direct conflict, a subset of the assertions passed to the constructor.
   */
  const Node& getConflict() const { return *d_conflict; }
  /**
   * Postprocess any lemma, replacing any newly created atom by the conjunction of its origins.
   */
  Node postprocessLemma(TNode lemma) { return lemma.substitute(d_lemmaSubstitutions); }

 private:
  /**
   * Main simplification method. First identify input equalities and compute their Gröbner basis.
   * Then store the simplified set of equalities back to d_simplified and d_lemmaSubstitutions.
   * Finally reduce all inequalities w.r.t. to the Gröbner basis and store the results.
   */
  void simplify();

  /**
   * Add a simplified assertion to d_simplified and add `simplified -> origins` to d_lemmaSubstitutions.
   * `simplified` is assumed to be an atom, while `origins` is assumed to be a conjunction of input assertions.
   * If `simplified` rewrited to true, it is ignored.
   * If `simplified` rewrites to false, we set d_conflict = NOT(origins) instead.
   * If `simplified` is an input assertion (i.e. it was not actually simplified), no substitution is added to d_lemmaSubstitutions.
   * Return false if a conflict was detected (and we can abort the process immediately), true otherwise.
   */
  bool addSimplification(TNode simplified, TNode origins);

  /** the list of input assertions */
  std::vector<Node> d_inputs;
  /** the simplified list of assertions */
  std::vector<Node> d_simplified;
  /** the conflict, if one has been found */
  std::optional<Node> d_conflict;
  /** the mapping of simplifications, suitable for Node::substitute(). Is used to reformulate lemmas in terms of the original input constraints. */
  std::unordered_map<TNode, TNode> d_lemmaSubstitutions;
  /** some internal state that abstracts away libpoly related utilities */
  std::unique_ptr<NGSState> d_state;
};

}  // namespace cvc5::theory::arith::nl::coverings

#endif
#endif
