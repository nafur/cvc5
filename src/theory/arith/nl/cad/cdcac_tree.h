/*********************                                                        */
/*! \file cdcac_tree.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Gereon Kremer
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2020 by the authors listed in the file AUTHORS
 ** in the top-level source directory and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Implements the CDCAC approach.
 **
 ** Implements the CDCAC approach as described in
 ** https://arxiv.org/pdf/2003.05633.pdf.
 **/

#include "cvc4_private.h"

#ifndef CVC4__THEORY__ARITH__NL__CAD__CDCAC_TREE_H
#define CVC4__THEORY__ARITH__NL__CAD__CDCAC_TREE_H

#include "util/real_algebraic_number.h"

#ifdef CVC4_POLY_IMP

#include <memory>
#include <vector>

#include "theory/arith/nl/cad/cdcac_utils.h"
#include "util/maybe.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

class CDCACTree {
public:
    struct TreeNode {
        TreeNode* parent;
        poly::Value sample;
        std::vector<CACInterval> intervals;
        std::vector<CACInterval> disabled_intervals;
        std::vector<std::unique_ptr<TreeNode>> children;

        std::vector<std::unique_ptr<TreeNode>>::const_iterator begin() const {
            return children.begin();
        }
        std::vector<std::unique_ptr<TreeNode>>::const_iterator end() const {
            return children.end();
        }

        TreeNode* addChild(const poly::Value& s) {
            children.emplace_back(new TreeNode{this, s, {}, {}});
            return children.back().get();
        }
        TreeNode* addChild(const poly::Value& s, const CACInterval& i) {
            TreeNode* res = addChild(s);
            res->intervals.emplace_back(i);
            return res;
        }

        void check_intervals(const std::vector<Node>& a)
        {
          std::vector<CACInterval> to_disable;
          std::remove_if(intervals.begin(),
                         intervals.end(),
                         [&to_disable, &a](const CACInterval& i) {
                           if (!std::includes(a.begin(),
                                              a.end(),
                                              i.d_origins.begin(),
                                              i.d_origins.end()))
                           {
                             to_disable.emplace_back(i);
                             return true;
                           }
                           return false;
                         });
          std::remove_if(disabled_intervals.begin(),
                         disabled_intervals.end(),
                         [this, &a](const CACInterval& i) {
                           if (std::includes(a.begin(),
                                             a.end(),
                                             i.d_origins.begin(),
                                             i.d_origins.end()))
                           {
                             intervals.emplace_back(i);
                             return true;
                           }
                           return false;
                         });
          disabled_intervals.insert(
              disabled_intervals.end(), to_disable.begin(), to_disable.end());
        }
    };

    TreeNode* sampleOutside(TreeNode* node) {
        std::vector<CACInterval> infeasible;
        for (const auto& child : *node)
        {
          infeasible.insert(infeasible.end(),
                            child->intervals.begin(),
                            child->intervals.end());
        }
        cleanIntervals(infeasible);
        for (const auto& child : *node)
        {
          if (!child->intervals.empty()) continue;
          bool free = std::none_of(infeasible.begin(),
                                   infeasible.end(),
                                   [&child](const CACInterval& i) {
                                     return poly::contains(i.d_interval, child->sample);
                                   });
          if (free)
          {
            return child.get();
          }
        }
        poly::Value sample;
        if (cad::sampleOutside(infeasible, sample)) {
            Trace("nl-cad") << "Sampled " << sample << std::endl;
            return node->addChild(sample);
        }
        return nullptr;
    }
    TreeNode* getRoot() {
        return &d_root;
    }
    const TreeNode* getRoot() const {
        return &d_root;
    }
    void check_intervals(std::vector<Node> assertions) {
        std::sort(assertions.begin(), assertions.end());
        d_root.check_intervals(assertions);
    }
private:
    TreeNode d_root;
};

std::ostream& operator<<(std::ostream& os, const CDCACTree::TreeNode& n);
std::ostream& operator<<(std::ostream& os, const CDCACTree& t);


}
}
}
}
}

#endif
#endif
