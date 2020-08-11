/*********************                                                        */
/*! \file variable_bounds.h
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

#ifndef CVC4__THEORY__ARITH__ICP__VARIABLE_BOUNDS_H
#define CVC4__THEORY__ARITH__ICP__VARIABLE_BOUNDS_H

#include <poly/polyxx.h>

#include <memory>
#include <vector>

#include "expr/node.h"
#include "theory/arith/nl/poly_conversion.h"
#include "theory/arith/normal_form.h"
#include "util/poly_util.h"

#include "theory/arith/nl/icp/interval.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace icp {

class VariableBounds
{
  VariableMapper& mMapper;
  std::map<Node, Interval> mIntervals;

  void update_lower_bound(const Node& origin,
                          const Node& variable,
                          const poly::Value& value,
                          bool strict);
  void update_upper_bound(const Node& origin,
                          const Node& variable,
                          const poly::Value& value,
                          bool strict);

 public:
  VariableBounds(VariableMapper& mapper) : mMapper(mapper) {}

  const VariableMapper& getMapper() const { return mMapper; }

  Interval& get(const Node& v);
  Interval get(const Node& v) const;

  std::vector<Node> getOrigins() const;

  poly::IntervalAssignment get() const;
  bool add(const Node& n);
};

std::ostream& operator<<(std::ostream& os, const VariableBounds& vb);

}  // namespace icp
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
