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
 * Node utilities for the arithmetic rewriter.
 */

#include "cvc5_private.h"

#ifndef CVC5__THEORY__ARITH__REWRITER__NODE_UTILS_H
#define CVC5__THEORY__ARITH__REWRITER__NODE_UTILS_H

#include "base/check.h"
#include "expr/node.h"

namespace cvc5::theory::arith::rewriter {


/** Make a nonlinear multiplication from the given factors */
template <typename T>
Node mkMult(T&& factors)
{
  auto* nm = NodeManager::currentNM();
  switch (factors.size())
  {
    case 0: return nm->mkConstInt(Rational(1));
    case 1: return factors[0];
    default: return nm->mkNode(Kind::NONLINEAR_MULT, std::forward<T>(factors));
  }
}

/** Make a sum from the given summands */
template <typename T>
Node mkSum(T&& summands)
{
  auto* nm = NodeManager::currentNM();
  switch (summands.size())
  {
    case 0: return nm->mkConstInt(Rational(0));
    case 1: return summands[0];
    default: return nm->mkNode(Kind::PLUS, std::forward<T>(summands));
  }
}

}

#endif