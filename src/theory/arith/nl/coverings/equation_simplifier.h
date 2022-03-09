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

#ifndef CVC5__THEORY__ARITH__NL__COVERINGS__EQUATION_SIMPLIFIER_H
#define CVC5__THEORY__ARITH__NL__COVERINGS__EQUATION_SIMPLIFIER_H

#include <memory>
#include <optional>
#include <unordered_map>

#include "expr/node.h"
#include "smt/env_obj.h"
#include "theory/arith/nl/nl_model.h"
#include "theory/substitutions.h"

namespace cvc5::theory::arith::nl::coverings {

/**
 * This class implements techniques to simplify arithmetic assertions based on
 * arithmetic equations. It does so by identifying term substitutions and atom
 * substitutions. These are used to simplify the set of assertions, possibly
 * identify conflicts, and postprocess both the model and lemmas that are
 * computed based on the simplified assertions.
 *
 * Term substitutions are used to effectively eliminate an arithmetic leaf node
 * t using an equation of the form `t = rest` (where rest does not contain t).
 * For every such substitution, t will be completely eliminated from the
 * simplified assertions and `t -> rest` is added to the model in
 * postprocessing.
 *
 * Atom substitutions are used to replace an assertion by a "simpler" assertion
 * (for some notion of "simpler"). The reason can either be a term substitution,
 * or a simplification with respect to another equation (e.g. a Gröbner-style
 * reduction). For every such substitution, the simplified atom will be replaced
 * by its origins (all input assertions that contributed to it) in conflicts and
 * lemmas. This technique is only available if both libpoly and CoCoALib are
 * available.
 *
 * Currently, two techniques are implemented:
 * - term elimination searches for equalities suitable for term substitution
 *   and performs the appropriate eliminations.
 * - Gröbner reduction computes the Gröbner basis of the equations. The
 *   equalities are replaced by the polynomials from the Gröbner basis, while
 *   all inequalities are reduced with respect to the ideal of the basis.
 *
 * The general usage should be as follows:
 * - EquationSimplifier(env, assertions)
 * - if (hasConflict()) emit getConflict()
 * - solve getSimplified()
 * - if (SAT) postprocessModel(model)
 * - else postprocessUnsatCore(conflict)
 */
class EquationSimplifier : protected EnvObj
{
 public:
  friend std::ostream& operator<<(std::ostream& os,
                                  const EquationSimplifier& ngs);

  /**
   * Holds all data used for the Gröbner reduction, in particular data from
   * libpoly and CoCoALib.
   */
  struct GroebnerState;

  /** Construct and perform simplification */
  EquationSimplifier(Env& env, const std::vector<Node>& inputs);
  /** Destruct */
  ~EquationSimplifier();

  /**
   * Return the simplified list of assertions. Should only be called if
   * hasConflict() returns false.
   */
  const std::vector<Node>& getSimplified() const { return d_simplified; }

  /** Check whether a direct conflict has been found */
  bool hasConflict() const { return d_conflict.has_value(); }
  /**
   * Return the conflict, which can directly be used as conflicting lemma.
   */
  const Node& getConflict() const { return *d_conflict; }
  /**
   * Postprocess an unsat core, replacing any atom by the conjunction of its
   * origins.
   */
  std::vector<Node> postprocessUnsatCore(const std::vector<Node>& core) const
  {
    return resolveAtomOrigins(core);
  }
  /**
   * Postprocess a model, adding substitutions for terms that have been
   * eliminated.
   */
  void postprocessModel(NlModel& model) const
  {
    for (const auto& sub : d_termSubstitutions)
    {
      model.addSubstitution(sub.first, sub.second);
    }
  }

 private:
  /**
   * Assumes that some assertion has been simplified to false and the respective
   * entry as been added to d_atomOrigins. Sets d_conflict appropriately.
   */
  void setConflict();

  /**
   * Main simplification method. Calls the submethods simplifyBy* in order.
   */
  void simplify();

  /**
   * Simplify by eliminating terms from simple equalities.
   */
  void simplifyByTermElimination(std::vector<Node>& equalities,
                                 std::vector<Node>& inequalities);
  /**
   * Simplify by reducing atoms w.r.t. to the Gröbner basis of the equalities.
   */
  void simplifyByGroebnerReduction(std::vector<Node>& equalities,
                                   std::vector<Node>& inequalities);

  /**
   * Utility class to collect atom origins. Takes care of resolving the
   * transitive origins from both d_termOrigins and d_atomOrigins.
   * Thus, any atoms (either input assertions or simplified atoms) as well as
   * eliminated terms (as collected by the tracker of SubstitutionMap::apply)
   * can be provided as origins.
   */
  struct AtomOriginBuilder
  {
    /** reference to main class */
    EquationSimplifier& d_target;
    /** simplified atom */
    const TNode d_simplified;
    /** origins collected so far */
    std::set<Node> d_origins;
    /** automatically commit the simplification when the utility is destroyed */
    ~AtomOriginBuilder();
    /** Add a single origin */
    AtomOriginBuilder& operator<<(TNode n);
    /** Add a single origin */
    AtomOriginBuilder& operator<<(const Node& n);
    /** Add multiple origins */
    AtomOriginBuilder& operator<<(const std::vector<Node>& n);
    /** Add multiple origins */
    AtomOriginBuilder& operator<<(const std::set<TNode>& n);
  };
  /**
   * Return the origin builder utility for the given simplified atom. The
   * simplification origins are committed to d_atomOrigins once the utility is
   * destroyed. Use like this:
   *
   * addToAtomOrigins(simp) << originalIneq << tracker;
   */
  AtomOriginBuilder addToAtomOrigins(TNode simplified);

  /** Resolve proper origins based on d_atomOrigins */
  std::vector<Node> resolveAtomOrigins(const std::vector<Node>& origins) const;

  /** the list of input assertions */
  std::vector<Node> d_inputs;
  /** the simplified list of assertions */
  std::vector<Node> d_simplified;
  /** the conflict, if one has been found */
  std::optional<Node> d_conflict;

  /** the term substitutions, stored as SubstitutionMap. */
  SubstitutionMap d_termSubstitutions;
  /** the origins of term substitutions. */
  std::unordered_map<Node, Node> d_termOrigins;
  /** the origins of all atom substitutions. */
  std::unordered_map<Node, std::vector<Node>> d_atomOrigins;
  /** some internal state that abstracts away libpoly and CoCoALib data */
  std::unique_ptr<GroebnerState> d_state;
};

std::ostream& operator<<(std::ostream& os, const EquationSimplifier& ngs);

}  // namespace cvc5::theory::arith::nl::coverings

#endif
