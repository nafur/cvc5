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

#include <poly/upolynomial.h>

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

  bool CadSolver::assign_model_variable(const Node& variable, const libpoly::Value& value) const {
    auto* nm = NodeManager::currentNM();
    switch (value.get()->type) {
      case LP_VALUE_INTEGER: {
        d_model.addCheckModelSubstitution(
          variable,
          nm->mkConst(Rational(libpoly::as_cvc_integer(&value.get()->value.z)))
        );
        return true;
      }
      case LP_VALUE_RATIONAL: {
        d_model.addCheckModelSubstitution(
          variable,
          nm->mkConst(libpoly::as_cvc_rational(&value.get()->value.q))
        );
        return true;
      }
      case LP_VALUE_DYADIC_RATIONAL: {
        d_model.addCheckModelSubstitution(
          variable,
          nm->mkConst(libpoly::as_cvc_rational(&value.get()->value.dy_q))
        );
        return true;
      }
      case LP_VALUE_ALGEBRAIC: {
        //Trace("cad-check") << value << " is an algebraic" << std::endl;
        // For the sake of it...
        const lp_algebraic_number_t& a = value.get()->value.a;
        for (std::size_t i = 0; i < 10; ++i) {
          lp_algebraic_number_refine_const(&a);
        }
        if (a.I.is_point) {
          d_model.addCheckModelSubstitution(
            variable,
            nm->mkConst(libpoly::as_cvc_rational(&a.I.a))
          );
        } else {
          Node poly = libpoly::as_cvc_polynomial(libpoly::UPolynomial(lp_upolynomial_construct_copy(a.f)), variable);
          // Construct witness:
          // a.f(x) == 0  &&  a.I.a < x  &&  x < a.I.b
          Node witness = nm->mkNode(Kind::AND,
            nm->mkNode(Kind::EQUAL, poly, nm->mkConst(Rational(0))),
            nm->mkNode(Kind::LT,
              nm->mkConst(libpoly::as_cvc_rational(&a.I.a)),
              variable
            ),
            nm->mkNode(Kind::LT,
              variable,
              nm->mkConst(libpoly::as_cvc_rational(&a.I.b))
            )
          );
          Trace("cad-check") << "Adding witness: " << witness << std::endl;
          d_model.addCheckModelWitness(variable, witness);
        }
        return true;
      }
      default: {
        Trace("cad-check") << value << " is weird" << std::endl;
        return false;
      }
    }
  }

  bool CadSolver::construct_model() {
    for (const auto& v: mCAC.get_variable_ordering()) {
      libpoly::Value val = mCAC.get_model().retrieve(v);
      Node variable = mCAC.get_constraints().var_mapper()(v);
      if (assign_model_variable(variable, val)) {
        Trace("cad-check") << "-> " << v << " = " << val << std::endl;
      } else {
        Trace("cad-check") << "Failed to set " << v << " = " << val << std::endl;
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
  Chat() << "CadSolver::checkInitialRefine" << std::endl;
  std::vector<Node> lems;
  
  // add lemmas corresponding to easy conflicts or refinements based on
  // the assertions/terms given in initLastCall.

  return lems;
}

std::vector<Node> CadSolver::checkFullRefine()
{
  Notice() << "CadSolver::checkFullRefine" << std::endl;
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
}

}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
