/******************************************************************************
 * Top contributors (to current version):
 *   Gereon Kremer, Aina Niemetz
 *
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2021 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The Interval class.
 */

#include "cvc5_private.h"

#ifndef CVC5__INTERVAL_EVALUATION_H
#define CVC5__INTERVAL_EVALUATION_H

#include <map>

#include "expr/node.h"
#include "util/interval.h"

namespace cvc5 {

Interval evaluate(TNode expression, const std::map<Node, Interval>& assignment);

}  // namespace cvc5

#endif /* CVC5__INTERVAL_EVALUATION_H */
