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
 * The Interval class.
 */

#include "util/interval_evaluation.h"

#include <functional>

namespace cvc5 {

namespace {

const Interval& getCached(TNode expression, const std::unordered_map<TNode, Interval>& cache)
{
    auto cit = cache.find(expression);
    Assert(cit != cache.end()) << "Node " << expression << " was not evaluated yet";
    return cit->second;
}

template<typename F>
Interval evaluate_nary(TNode expression, const std::unordered_map<TNode, Interval>& cache, F&& aggregator) {
    Interval res = getCached(expression[0], cache);
    for (size_t i = 1; i < expression.getNumChildren(); ++i) {
        res = aggregator(res, getCached(expression[i], cache));
    }
    return res;
}

Interval evaluate_impl(TNode expression, const std::unordered_map<TNode, Interval>& cache)
{
    switch (expression.getKind()) {
        case Kind::PLUS:

            return evaluate_nary(expression, cache, std::plus<>());
        case Kind::MULT:
        case Kind::NONLINEAR_MULT:
            return evaluate_nary(expression, cache, std::multiplies<>());
        case Kind::MINUS:
            return evaluate_nary(expression, cache, std::minus<>());
        case Kind::UMINUS:
            return -getCached(expression[0], cache);
        case Kind::DIVISION:
        case Kind::DIVISION_TOTAL:
            return evaluate_nary(expression, cache, std::divides<>());
        case Kind::ABS:
            return abs(getCached(expression[0], cache));
        case Kind::POW:
            return abs(getCached(expression[0], cache));
        case Kind::EXPONENTIAL:
            return exp(getCached(expression[0], cache));
        case Kind::SINE:
            return sin(getCached(expression[0], cache));
        case Kind::COSINE:
            return cos(getCached(expression[0], cache));
        case Kind::TANGENT:
            return tan(getCached(expression[0], cache));
        case Kind::COSECANT:
            return cosec(getCached(expression[0], cache));
        case Kind::SECANT:
            return sec(getCached(expression[0], cache));
        case Kind::COTANGENT:
            return cotan(getCached(expression[0], cache));
        case Kind::ARCSINE:
            return asin(getCached(expression[0], cache));
        case Kind::ARCCOSINE:
            return acos(getCached(expression[0], cache));
        case Kind::ARCTANGENT:
            return atan(getCached(expression[0], cache));
        case Kind::ARCCOSECANT:
            return acosec(getCached(expression[0], cache));
        case Kind::ARCSECANT:
            return asec(getCached(expression[0], cache));
        case Kind::ARCCOTANGENT:
            return acotan(getCached(expression[0], cache));
        case Kind::SQRT:
            return sqrt(getCached(expression[0], cache));
        case Kind::PI:
            return Interval::pi();
        default:
            Assert(false) << "Can only evaluate arithmetic nodes over intervals, but got " << expression << std::endl;
            return Interval(0.0, 0.0);
    }
}

}

Interval evaluate(TNode expression, const std::map<Node, Interval>& assignment)
{
    std::unordered_map<TNode, Interval> cache;
    std::vector<TNode> queue = { expression };
    while (!queue.empty()) {
        TNode cur = queue.back();
        if (cache.find(cur) != cache.end()) {
            queue.pop_back();
            continue;
        }
        auto ait = assignment.find(cur);
        if (ait != assignment.end()) {
            cache.emplace(cur, ait->second);
            queue.pop_back();
            continue;
        }
        bool doProcess = true;
        for (const auto& child: cur) {
            auto cit = cache.find(child);
            if (cit == cache.end()) {
                queue.emplace_back(child);
                doProcess = false;
            }
        }
        if (doProcess) {
            cache.emplace(cur, evaluate_impl(cur, cache));
            queue.pop_back();
        }
    }
    auto cit = cache.find(expression);
    Assert(cit != cache.end());
    return cit->second;
}

}  // namespace cvc5
