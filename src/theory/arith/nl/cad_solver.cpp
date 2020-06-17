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

//#include <poly/upolynomial.h>

#include "theory/arith/nl/cad_solver.h"

#include <poly/polyxx.h>

#include "options/arith_options.h"
#include "options/smt_options.h"
#include "preprocessing/passes/bv_to_int.h"
#include "theory/arith/arith_msum.h"
#include "theory/arith/arith_utilities.h"
#include "theory/arith/nl/cad/cdcac.h"
#include "theory/arith/nl/poly_conversion.h"
#include "util/poly_util.h"

using namespace CVC4::kind;

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {

bool CadSolver::construct_model()
{
  for (const auto& v : mCAC.get_variable_ordering())
  {
    Node variable = mCAC.get_constraints().var_mapper()(v);
    Node value = value_to_node(mCAC.get_model().get(v));
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

CadSolver::CadSolver(TheoryArith& containing, NlModel& model)
    : d_containing(containing), d_model(model)
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
  mCAC.reset();
  for (const Node& a : assertions)
  {
    mCAC.get_constraints().add_constraint(a);
  }
  mCAC.compute_variable_ordering();
}

std::vector<NlLemma> CadSolver::checkInitialRefine()
{
  Chat() << "CadSolver::checkInitialRefine" << std::endl;
  std::vector<NlLemma> lems;

  // add lemmas corresponding to easy conflicts or refinements based on
  // the assertions/terms given in initLastCall.

  return lems;
}

std::vector<NlLemma> CadSolver::checkFullRefine()
{
  Notice() << "CadSolver::checkFullRefine" << std::endl;
  std::vector<NlLemma> lems;

  // Do full theory check here

  auto covering = mCAC.get_unsat_cover();
  if (covering.empty())
  {
    found_satisfiability = true;
    Notice() << "SAT: " << mCAC.get_model() << std::endl;
  }
  else
  {
    found_satisfiability = false;
    auto mis = collect_constraints(covering);
    Notice() << "Collected MIS: " << mis << std::endl;
    auto* nm = NodeManager::currentNM();
    for (auto& n : mis)
    {
      n = n.negate();
    }
    Assert(!mis.empty()) << "Infeasible subset can not be empty";
    if (mis.size() == 1)
    {
      lems.emplace_back(mis.front());
    }
    else
    {
      lems.emplace_back(nm->mkNode(Kind::OR, mis));
    }
    Notice() << "UNSAT with MIS: " << lems.back().d_lemma << std::endl;
  }

  return lems;
}

void CadSolver::preprocessAssertionsCheckModel(std::vector<Node>& assertions)
{
  if (found_satisfiability)
  {
    Notice() << "Storing " << mCAC.get_model() << std::endl;
    construct_model();
    assertions.clear();
  }
}

}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
