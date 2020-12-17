/*********************                                                        */
/*! \file cdcac_tree.cpp
 ** \verbatim
 ** Top contributors (to current version):
 **   Gereon Kremer
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2020 by the authors listed in the file AUTHORS
 ** in the top-level source directory and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Implements utilities for cdcac.
 **
 ** Implements utilities for cdcac.
 **/

#include "theory/arith/nl/cad/cdcac_tree.h"

#ifdef CVC4_POLY_IMP

#include "theory/arith/nl/cad/projections.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

void CDCACTree::TreeNode::check_intervals(const std::vector<Node>& a)
{
  std::vector<CACInterval> to_disable;
  {
    auto it = std::remove_if(
        intervals.begin(),
        intervals.end(),
        [&to_disable, &a](const CACInterval& i) {
          if (!std::includes(
                  a.begin(), a.end(), i.d_origins.begin(), i.d_origins.end()))
          {
            Trace("cdcac-tree") << "Disabling " << i.d_interval << std::endl;
            to_disable.emplace_back(i);
            return true;
          }
          return false;
        });
    intervals.erase(it, intervals.end());
  }
  {
    auto it = std::remove_if(
        disabled_intervals.begin(),
        disabled_intervals.end(),
        [this, &a](const CACInterval& i) {
          if (std::includes(
                  a.begin(), a.end(), i.d_origins.begin(), i.d_origins.end()))
          {
            Trace("cdcac-tree") << "Enabling " << i.d_interval << std::endl;
            intervals.emplace_back(i);
            return true;
          }
          return false;
        });
    disabled_intervals.erase(it, disabled_intervals.end());
  }
  disabled_intervals.insert(
      disabled_intervals.end(), to_disable.begin(), to_disable.end());
  for (auto& child : children)
  {
    child->check_intervals(a);
  }
}

CDCACTree::TreeNode* CDCACTree::sampleOutside(TreeNode* node)
{
  Assert(node != nullptr);
  std::vector<CACInterval> infeasible;
  for (const auto& child : *node)
  {
    infeasible.insert(
        infeasible.end(), child->intervals.begin(), child->intervals.end());
  }
  cleanIntervals(infeasible);
  for (const auto& child : *node)
  {
    if (child->sample.nothing()) continue;
    if (!child->intervals.empty()) continue;
    bool free = std::none_of(
        infeasible.begin(), infeasible.end(), [&child](const CACInterval& i) {
          if (child->sample.nothing()) return true;
          return poly::contains(i.d_interval, child->sample.value());
        });
    if (free)
    {
      Trace("nl-cad") << "Use existing " << child.get() << std::endl;
      return child.get();
    }
  }
  Trace("nl-cad") << "Sampling new value" << std::endl;
  poly::Value sample;
  if (cad::sampleOutside(infeasible, sample))
  {
    Trace("nl-cad") << "Sampled " << sample << std::endl;
    return node->addChild(sample);
  }
  return nullptr;
}

void CDCACTree::check_intervals(std::vector<Node> assertions)
{
  Trace("cdcac-tree") << "Checking existing intervals based on new assertions"
            << std::endl;
  Trace("cdcac-tree") << "Before: " << std::endl << *this << std::endl;
  std::sort(assertions.begin(), assertions.end());
  d_root.check_intervals(assertions);
  Trace("cdcac-tree") << "After: " << std::endl << *this << std::endl;
}

std::ostream& operator<<(std::ostream& os, const CDCACTree::TreeNode& n) {
    os << n.sample << " @ " << static_cast<const void*>(&n);
    for (const auto& i: n.intervals) {
        os << " " << i.d_interval << " {";
        for (const auto& p: i.d_origins) os << " " << p;
        os << " }";
    }
    return os;
}
inline void print(std::ostream& os, const CDCACTree::TreeNode* n, std::size_t indent) {
    for (std::size_t i = 0; i < indent; ++i) os << '\t';
    os << *n << std::endl;
    for (const auto& child: *n) {
        print(os, child.get(), indent + 1);
    }
}
std::ostream& operator<<(std::ostream& os, const CDCACTree& t) {
    os << "CDCAC Tree" << std::endl;
    print(os, t.getRoot(), 0);
    return os;
}

}
}
}
}
}

#endif
