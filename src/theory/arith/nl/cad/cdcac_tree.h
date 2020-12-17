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
        Maybe<poly::Value> sample;
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
        void addDirectConflict(const CACInterval& i) {
            for (const auto& c: children) {
                if (c->sample) continue;
                if (c->intervals.size() == 1 && c->intervals[0] == i)
                    return;
            }
            children.emplace_back(new TreeNode{this, Maybe<poly::Value>(), {}, {}});
            children.back()->intervals.emplace_back(i);
        }

        void check_intervals(const std::vector<Node>& a);
    };

    TreeNode* sampleOutside(TreeNode* node);
    TreeNode* getRoot() {
        return &d_root;
    }
    const TreeNode* getRoot() const {
        return &d_root;
    }
    void check_intervals(std::vector<Node> assertions);
    void clear() {
      d_root = TreeNode();
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
