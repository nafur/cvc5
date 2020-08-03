/*********************                                                        */
/*! \file cad_solver.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Andrew Reynolds
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Implementation of new non-linear solver
 **/

#include "theory/arith/nl/cad_solver.h"

#include <poly/polyxx.h>

#include "inference.h"
#include "theory/arith/nl/cad/cdcac.h"
#include "theory/arith/nl/poly_conversion.h"
#include "util/poly_util.h"
#include "cad/theory_call_exporter.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {

// #define EXPORT_THEORY_CALLS

bool CadSolver::constructModelIfAvailable(std::vector<Node>& assertions)
{
  if (d_foundSatisfiability) {
    assertions.clear();
    for (const auto& v : d_CAC.getVariableOrdering())
    {
      Node variable = d_CAC.getConstraints().varMapper()(v);
      Node value = value_to_node(d_CAC.getModel().get(v), d_ranVariable);
      if (value.isConst())
      {
        d_model.addCheckModelSubstitution(variable, value);
      }
      else
      {
        d_model.addCheckModelWitness(variable, value);
      }
      Trace("cad-check") << "-> " << v << " = " << value << std::endl;
    }
    return true;
  }
  return false;
}

CadSolver::CadSolver(TheoryArith& containing, NlModel& model)
    : d_ranVariable(NodeManager::currentNM()->mkSkolem("__z", NodeManager::currentNM()->realType(), "", NodeManager::SKOLEM_EXACT_NAME)), d_containing(containing), d_model(model)
{
}

CadSolver::~CadSolver() {}

void CadSolver::initLastCall(const std::vector<Node>& assertions,
                             const std::vector<Node>& false_asserts,
                             const std::vector<Node>& xts)
{
  if (Trace.isOn("cad-check"))
  {
    Trace("cad-check") << "CadSolver::initLastCall" << std::endl;
    Trace("cad-check") << "* Assertions: " << std::endl;
    for (const Node& a : assertions)
    {
      Trace("cad-check") << "  " << a << std::endl;
      if (std::find(false_asserts.begin(), false_asserts.end(), a)
          != false_asserts.end())
      {
        Trace("cad-check") << " (false in candidate model)" << std::endl;
      }
    }
    Trace("cad-check") << "* Extended terms: " << std::endl;
    for (const Node& t : xts)
    {
      Trace("cad-check") << "  " << t << std::endl;
    }
  }
  // store or process assertions
  d_CAC.reset();
  for (const Node& a : assertions)
  {
    d_CAC.getConstraints().addConstraint(a);
  }
#ifdef EXPORT_THEORY_CALLS
  static std::size_t theory_calls = 0;
  ++theory_calls;
  cad::NRAFeatures stats(d_CAC.get_constraints().get_constraints());
  cad::export_theory_call(theory_calls, assertions, stats);
#endif
  d_CAC.computeVariableOrdering();
  d_CAC.retrieve_initial_assignment(d_model, d_ranVariable);
}

std::vector<NlLemma> CadSolver::checkFull()
{
#ifdef EXPORT_THEORY_CALLS
  std::cout << "Abort solving as we only export theory calls." << std::endl;
  return {};
#endif

  std::vector<NlLemma> lems;
  auto covering = d_CAC.getUnsatCover();
  if (covering.empty())
  {
    d_foundSatisfiability = true;
    Notice() << "SAT: " << d_CAC.getModel() << std::endl;
  }
  else
  {
    d_foundSatisfiability = false;
    auto mis = collectConstraints(covering);
    Notice() << "Collected MIS: " << mis << std::endl;
    auto* nm = NodeManager::currentNM();
    for (auto& n : mis)
    {
      n = n.negate();
    }
    Assert(!mis.empty()) << "Infeasible subset can not be empty";
    if (mis.size() == 1)
    {
      lems.emplace_back(mis.front(), Inference::CAD_CONFLICT);
    }
    else
    {
      lems.emplace_back(nm->mkNode(Kind::OR, mis), Inference::CAD_CONFLICT);
    }
    Notice() << "UNSAT with MIS: " << lems.back().d_lemma << std::endl;
  }
  return lems;
}

std::vector<NlLemma> CadSolver::checkPartial()
{
  std::vector<NlLemma> lems;
  auto covering = d_CAC.getUnsatCover(0, true);
  if (covering.empty())
  {
    d_foundSatisfiability = true;
    Notice() << "SAT: " << d_CAC.getModel() << std::endl;
  }
  else
  {
    for (const auto& interval: covering) {
      Node first_var = d_CAC.getConstraints().varMapper()(d_CAC.getVariableOrdering()[0]);
      Node lemma = excluding_interval_to_lemma(first_var, interval.d_interval);
      lems.emplace_back(lemma, Inference::CAD_EXCLUDED_INTERVAL);
    }
  }
  return lems;
}

}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
