/*********************                                                        */
/*! \file icp.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Gereon Kremer
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief
 **/

#include "theory/arith/nl/icp/icp.h"

#include <iostream>

#include "base/check.h"
#include "base/output.h"
#include "theory/arith/nl/poly_conversion.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace icp {

std::vector<Node> ICPSolver::collectVariables(const Node& n) const
{
  std::unordered_set<TNode, TNodeHashFunction> tmp;
  expr::getVariables(n, tmp);
  std::vector<Node> res;
  for (const auto& t : tmp)
  {
    res.emplace_back(t);
  }
  return res;
}

Maybe<Candidate> ICPSolver::constructCandidate(const Node& n)
{
  auto comp = Comparison::parseNormalForm(n).decompose(false);
  Kind k = std::get<1>(comp);
  if (k == Kind::DISTINCT)
  {
    return Maybe<Candidate>();
  }
  auto poly = std::get<0>(comp);

  std::unordered_set<TNode, TNodeHashFunction> vars;
  expr::getVariables(n, vars);
  for (const auto& v : vars)
  {
    Trace("nl-icp") << "\tChecking " << n << " for " << v << std::endl;

    std::map<Node, Node> msum;
    ArithMSum::getMonomialSum(poly.getNode(), msum);

    Node veq_c;
    Node val;

    int isolated = ArithMSum::isolate(v, msum, veq_c, val, k);
    if (isolated == 1)
    {
      poly::Variable lhs = mMapper(v);
      poly::SignCondition rel;
      switch (k)
      {
        case Kind::LT: rel = poly::SignCondition::LT; break;
        case Kind::LEQ: rel = poly::SignCondition::LE; break;
        case Kind::EQUAL: rel = poly::SignCondition::EQ; break;
        case Kind::DISTINCT: rel = poly::SignCondition::NE; break;
        case Kind::GT: rel = poly::SignCondition::GT; break;
        case Kind::GEQ: rel = poly::SignCondition::GE; break;
        default: Assert(false) << "Unexpected kind: " << k;
      }
      poly::Rational rhsmult;
      poly::Polynomial rhs = as_poly_polynomial(val, mMapper, rhsmult);
      rhsmult = poly::Rational(1) / rhsmult;
      // only correct up to a constant (denominator is thrown away!)
      // std::cout << "rhs = " << rhs << std::endl;
      if (!veq_c.isNull())
      {
        rhsmult = poly_utils::toRational(veq_c.getConst<Rational>());
      }
      Candidate res{lhs, rel, rhs, rhsmult, n, collectVariables(val)};
      Trace("nl-icp") << "\tAdded " << res << " from " << n << std::endl;
      return res;
    }
    else if (isolated == -1)
    {
      poly::Variable lhs = mMapper(v);
      poly::SignCondition rel;
      switch (k)
      {
        case Kind::LT: rel = poly::SignCondition::GT; break;
        case Kind::LEQ: rel = poly::SignCondition::GE; break;
        case Kind::EQUAL: rel = poly::SignCondition::EQ; break;
        case Kind::DISTINCT: rel = poly::SignCondition::NE; break;
        case Kind::GT: rel = poly::SignCondition::LT; break;
        case Kind::GEQ: rel = poly::SignCondition::LE; break;
        default: Assert(false) << "Unexpected kind: " << k;
      }
      poly::Rational rhsmult;
      poly::Polynomial rhs = as_poly_polynomial(val, mMapper, rhsmult);
      rhsmult = poly::Rational(1) / rhsmult;
      if (!veq_c.isNull())
      {
        rhsmult = poly_utils::toRational(veq_c.getConst<Rational>());
      }
      Candidate res{lhs, rel, rhs, rhsmult, n, collectVariables(val)};
      Trace("nl-icp") << "\tAdded " << res << " from " << n << std::endl;
      return res;
    }
  }
  return Maybe<Candidate>();
}

void ICPSolver::addCandidate(const Node& n)
{
  auto it = mCandidateCache.find(n);
  if (it != mCandidateCache.end())
  {
    mState->mCandidates.emplace_back(it->second);
  }
  else
  {
    auto c = constructCandidate(n);
    if (c)
    {
      mCandidateCache.emplace(n, c.value());
      mState->mCandidates.emplace_back(c.value());
      mBudget += mBudgetIncrement;
    }
  }
}

void ICPSolver::reset() { mState.reset(new ICPState(mMapper)); }

void ICPSolver::add(const Node& n)
{
  Trace("nl-icp") << "Trying to add " << n << std::endl;
  if (!mState->mBounds.add(n))
  {
    addCandidate(n);
  }
}

void ICPSolver::init()
{
  for (const auto& vars : mMapper.mVarCVCpoly)
  {
    auto& i = mState->mBounds.get(vars.first);
    Trace("nl-icp") << "Adding initial " << vars.first << " -> " << i
                    << std::endl;
    if (!i.lower_origin.isNull())
    {
      Trace("nl-icp") << "\tAdding lower " << i.lower_origin << std::endl;
      mState->mOrigins.add(vars.first, i.lower_origin, {});
    }
    if (!i.upper_origin.isNull())
    {
      Trace("nl-icp") << "\tAdding upper " << i.upper_origin << std::endl;
      mState->mOrigins.add(vars.first, i.upper_origin, {});
    }
  }
}

PropagationResult ICPSolver::doIt(poly::IntervalAssignment& ia)
{
  if (mBudget <= 0)
  {
    Trace("nl-icp") << "ICP budget exceeded" << std::endl;
    return PropagationResult::NOT_CHANGED;
  }
  mState->mLastConflict = Node();
  Trace("nl-icp") << "Starting propagation with " << IAWrapper{ia, mMapper}
                  << std::endl;
  Trace("nl-icp") << "Current budget: " << mBudget << std::endl;
  PropagationResult res = PropagationResult::NOT_CHANGED;
  for (const auto& c : mState->mCandidates)
  {
    --mBudget;
    PropagationResult cres = c.propagate(ia);
    switch (cres)
    {
      case PropagationResult::NOT_CHANGED: break;
      case PropagationResult::CONTRACTED:
      case PropagationResult::CONTRACTED_STRONGLY:
        mState->mOrigins.add(mMapper(c.lhs), c.origin, c.rhsVariables);
        res = PropagationResult::CONTRACTED;
        break;
      case PropagationResult::CONTRACTED_WITHOUT_CURRENT:
      case PropagationResult::CONTRACTED_STRONGLY_WITHOUT_CURRENT:
        mState->mOrigins.add(mMapper(c.lhs), c.origin, c.rhsVariables, false);
        res = PropagationResult::CONTRACTED;
        break;
      case PropagationResult::CONFLICT:
        mState->mOrigins.add(mMapper(c.lhs), c.origin, c.rhsVariables);
        auto nm = NodeManager::currentNM();
        mState->mLastConflict = nm->mkNode(
            Kind::NOT,
            nm->mkNode(Kind::AND, mState->mOrigins.getOrigins(mMapper(c.lhs))));
        return PropagationResult::CONFLICT;
    }
    switch (cres)
    {
      case PropagationResult::CONTRACTED_STRONGLY:
      case PropagationResult::CONTRACTED_STRONGLY_WITHOUT_CURRENT:
        Trace("nl-icp") << "Bumping budget because of a strong contraction" << std::endl;
        mBudget += mBudgetIncrement;
        break;
      default: break;
    }
  }
  return res;
}

std::vector<Node> ICPSolver::asLemmas(const poly::IntervalAssignment& ia) const
{
  auto nm = NodeManager::currentNM();
  std::vector<Node> lemmas;

  for (const auto& vars : mMapper.mVarCVCpoly)
  {
    if (!ia.has(vars.second)) continue;
    Node v = vars.first;
    poly::Interval i = ia.get(vars.second);
    if (!is_minus_infinity(get_lower(i)))
    {
      Kind rel = get_lower_open(i) ? Kind::GT : Kind::GEQ;
      Node c = nm->mkNode(rel, v, value_to_node(get_lower(i), v));
      Node premise = nm->mkNode(Kind::AND, mState->mOrigins.getOrigins(v));
      Trace("nl-icp") << premise << " => " << c << std::endl;
      Node lemma = Rewriter::rewrite(nm->mkNode(Kind::IMPLIES, premise, c));
      if (lemma.isConst())
      {
        Assert(lemma == nm->mkConst<bool>(true));
      }
      else
      {
        Trace("nl-icp") << "Adding lemma " << lemma << std::endl;
        lemmas.emplace_back(lemma);
      }
    }
    if (!is_plus_infinity(get_upper(i)))
    {
      Kind rel = get_upper_open(i) ? Kind::LT : Kind::LEQ;
      Node c = nm->mkNode(rel, v, value_to_node(get_upper(i), v));
      Node premise = nm->mkNode(Kind::AND, mState->mOrigins.getOrigins(v));
      Trace("nl-icp") << premise << " => " << c << std::endl;
      Node lemma = Rewriter::rewrite(nm->mkNode(Kind::IMPLIES, premise, c));
      if (lemma.isConst())
      {
        Assert(lemma == nm->mkConst<bool>(true));
      }
      else
      {
        Trace("nl-icp") << "Adding lemma " << lemma << std::endl;
        lemmas.emplace_back(lemma);
      }
    }
  }
  return lemmas;
}

}  // namespace icp
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
