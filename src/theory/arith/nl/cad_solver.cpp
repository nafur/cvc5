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
#include "theory/arith/nl/cad/cdcac.h"
#include "theory/arith/nl/libpoly/conversion.h"

#include "options/arith_options.h"
#include "options/smt_options.h"
#include "preprocessing/passes/bv_to_int.h"
#include "theory/arith/arith_msum.h"
#include "theory/arith/arith_utilities.h"

using namespace CVC4::kind;

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {

  bool CadSolver::extract_bounds(const libpoly::Value& value, Node& lower, Node& upper) const {
    auto* nm = NodeManager::currentNM();
    switch (value.get()->type) {
      case LP_VALUE_INTEGER: {
        Trace("cad-check") << value << " is an integer" << std::endl;
        lower = nm->mkConst(Rational(libpoly::as_cvc_integer(&value.get()->value.z)));
        upper = lower;
        return true;
      }
      case LP_VALUE_RATIONAL: {
        Trace("cad-check") << value << " is a rational" << std::endl;
        lower = nm->mkConst(libpoly::as_cvc_rational(&value.get()->value.q));
        upper = lower;
        return true;
      }
      case LP_VALUE_DYADIC_RATIONAL: {
        Trace("cad-check") << value << " is a dyadic rational" << std::endl;
        lower = nm->mkConst(libpoly::as_cvc_rational(&value.get()->value.dy_q));
        upper = lower;
        return true;
      }
      case LP_VALUE_ALGEBRAIC: {
        Trace("cad-check") << value << " is an algebraic" << std::endl;
        lower = nm->mkConst(libpoly::as_cvc_rational(&value.get()->value.a.I.a));
        upper = nm->mkConst(libpoly::as_cvc_rational(&value.get()->value.a.I.b));
        return true;
      }
      default: {
        Trace("cad-check") << value << " is weird" << std::endl;
        return false;
      }
    }
  }

  bool CadSolver::construct_model() const {
    for (const auto& v: mCAC.get_variable_ordering()) {
      libpoly::Value val = mCAC.get_model().retrieve(v);
      std::cout << "-> " << v << " = " << val << std::endl;

      Node lower;
      Node upper;
      if (extract_bounds(val, lower, upper)) {
        Trace("cad-check") << "Extracted " << val << " in " << lower << " .. " << upper << std::endl;
        d_model.addCheckModelBound(
          mCAC.get_constraints().var_poly_to_cvc(v),
          lower, upper
        );
      }
    }
    return true;
  }

CadSolver::CadSolver(TheoryArith& containing, NlModel& model)
    : d_containing(containing),
      d_model(model)
{
  NodeManager* nm = NodeManager::currentNM();
  d_true = nm->mkConst(true);
  d_false = nm->mkConst(false);
  d_zero = nm->mkConst(Rational(0));
  d_one = nm->mkConst(Rational(1));
  d_neg_one = nm->mkConst(Rational(-1));
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
      if (std::find(false_asserts.begin(),false_asserts.end(),a)!=false_asserts.end())
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

std::vector<Node> CadSolver::checkInitialRefine()
{
  Trace("cad-check") << "CadSolver::checkInitialRefine" << std::endl;
  std::vector<Node> lems;
  
  // add lemmas corresponding to easy conflicts or refinements based on
  // the assertions/terms given in initLastCall.
  
  return lems;
}

std::vector<Node> CadSolver::checkFullRefine()
{
  Trace("cad-check") << "CadSolver::checkFullRefine";
  std::vector<Node> lems;

  // Do full theory check here

  auto covering = mCAC.get_unsat_cover();
  if (covering.empty()) {
    Notice() << "SAT: " << mCAC.get_model() << std::endl;
    construct_model();
  } else {
    auto mis = mCAC.collect_constraints(covering);
    auto* nm = NodeManager::currentNM();
    for (auto& n: mis) {
      n = n.negate();
    }
    lems.emplace_back(nm->mkNode(Kind::OR, mis));
    Notice() << "UNSAT with MIS: " << lems.back() << std::endl;
  } 
  
  return lems;
}

void CadSolver::preprocessAssertionsCheckModel(std::vector<Node>& assertions)
{
  Notice() << "##### Asking for model." << std::endl;
  // Report model into assertions
}

}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
