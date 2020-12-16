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


std::ostream& operator<<(std::ostream& os, const CDCACTree::TreeNode& n) {
    os << n.sample;
    for (const auto& i: n.intervals) {
        os << " " << i.d_interval;
    }
    return os;
}
inline void print(std::ostream& os, const CDCACTree::TreeNode* n, std::size_t indent) {
    for (std::size_t i = 0; i < indent; ++i) os << '\t';
    if (indent != 0) {
        os << *n << std::endl;
    }
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
