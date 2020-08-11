/*********************                                                        */
/*! \file candidate.h
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

#ifndef CVC4__THEORY__ARITH__ICP__CANDIDATE_H
#define CVC4__THEORY__ARITH__ICP__CANDIDATE_H

#include <poly/polyxx.h>

#include "expr/node.h"
#include "intersection.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace icp {


struct Candidate {
    poly::Variable lhs;
    poly::SignCondition rel;
    poly::Polynomial rhs;
    poly::Rational rhsmult;
    Node origin;
    std::vector<Node> rhsVariables;

    PropagationResult propagate(poly::IntervalAssignment& ia) const;
};
std::ostream& operator<<(std::ostream& os, const Candidate& c);

}  // namespace icp
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
