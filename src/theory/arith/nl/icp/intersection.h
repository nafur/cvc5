/*********************                                                        */
/*! \file intersection.h
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

#ifndef CVC4__THEORY__ARITH__ICP__INTERSECTION_H
#define CVC4__THEORY__ARITH__ICP__INTERSECTION_H

#include <poly/polyxx.h>

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace icp {

enum class PropagationResult
{
  NOT_CHANGED,
  CONTRACTED,
  CONTRACTED_STRONGLY,
  CONTRACTED_WITHOUT_CURRENT,
  CONTRACTED_STRONGLY_WITHOUT_CURRENT,
  CONFLICT
};

PropagationResult intersect_interval_with(poly::Interval& cur,
                                          const poly::Interval& res,
                                          std::size_t size_threshold);

}  // namespace icp
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
