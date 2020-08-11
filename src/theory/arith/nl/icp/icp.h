#ifndef CVC4__THEORY__ARITH__ICP__ICP_H
#define CVC4__THEORY__ARITH__ICP__ICP_H

#include "expr/node.h"
#include "expr/node_algorithm.h"
#include "theory/arith/arith_msum.h"
#include "theory/arith/normal_form.h"

#include "theory/rewriter.h"

#include <poly/polyxx.h>
#include "util/poly_util.h"
#include "theory/arith/nl/poly_conversion.h"

#include "theory/arith/nl/icp/candidate.h"
#include "theory/arith/nl/icp/contraction_origins.h"
#include "theory/arith/nl/icp/intersection.h"
#include "theory/arith/nl/icp/variable_bounds.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace icp {

struct IAWrapper {
    const poly::IntervalAssignment& ia;
    const VariableMapper& vm;
};
inline std::ostream& operator<<(std::ostream& os, const IAWrapper& iaw) {
    os << "{ ";
    bool first = true;
    for (const auto& v: iaw.vm.mVarpolyCVC) {
        if (iaw.ia.has(v.first)) {
            if (first) {
                first = false;
            } else {
                os << ", ";
            }
            os << v.first << " -> " << iaw.ia.get(v.first);
        }
    }
    return os << " }";
}

struct ICPState {
    VariableBounds mBounds;
    std::vector<Candidate> mCandidates;
    ContractionOriginManager mOrigins;
    Node mLastConflict;

    ICPState(VariableMapper& vm): mBounds(vm) {}
};

class ICPSolver {
    VariableMapper mMapper;
    std::map<Node, Candidate> mCandidateCache;
    std::unique_ptr<ICPState> mState;

    std::int64_t mBudget = 0;
    static constexpr std::int64_t mBudgetIncrement = 10;

    std::vector<Node> collectVariables(const Node& n) const;
    Maybe<Candidate> constructCandidate(const Node& n);
    void addCandidate(const Node& n);

public:
    void reset();
    void add(const Node& n);
    void init();

    poly::IntervalAssignment getInitial() const {
        return mState->mBounds.get();
    }

    Node getConflict() const {
        return mState->mLastConflict;
    }

    PropagationResult doIt(poly::IntervalAssignment& ia);
    std::vector<Node> asLemmas(const poly::IntervalAssignment& ia) const;

    void print() {
        std::cout << mState->mBounds << std::endl;
        std::cout << "Candidates:" << std::endl;
        for (const auto& c: mState->mCandidates) {
            std::cout << "\t" << c << std::endl;
        }
        std::cout << mState->mOrigins << std::endl;
    }
};

class Propagator {
    VariableMapper mMapper;
    VariableBounds mBounds;
    std::vector<Candidate> mCandidates;
    ContractionOriginManager mOrigins;
    Node mLastConflict;

    std::int64_t mBudget = 0;
    static constexpr std::int64_t mBudgetMultiplier = 10;

    std::vector<Node> collectVariables(const Node& n) const {
        std::unordered_set<TNode, TNodeHashFunction> tmp;
        expr::getVariables(n, tmp);
        std::vector<Node> res;
        for (const auto& t: tmp) {
            res.emplace_back(t);
        }
        return res;
    }

    void addCandidate(const Node& n) {
        auto comp = Comparison::parseNormalForm(n).decompose(false);
        Kind k = std::get<1>(comp);
        if (k == Kind::DISTINCT) {
            return;
        }
        auto poly = std::get<0>(comp);

        std::unordered_set<TNode, TNodeHashFunction> vars;
        expr::getVariables(n, vars);
        for (const auto& v: vars) {
            Trace("nl-icp") << "\tChecking " << n << " for " << v << std::endl;

            std::map<Node, Node> msum;
            ArithMSum::getMonomialSum(poly.getNode(), msum);

            Node veq_c;
            Node val;
            
            int res = ArithMSum::isolate(v, msum, veq_c, val, k);
            if (res == 1) {
                poly::Variable lhs = mMapper(v);
                poly::SignCondition rel;
                switch (k) {
                    case Kind::LT: rel = poly::SignCondition::LT; break;
                    case Kind::LEQ: rel = poly::SignCondition::LE; break;
                    case Kind::EQUAL: rel = poly::SignCondition::EQ; break;
                    case Kind::DISTINCT: rel = poly::SignCondition::NE; break;
                    case Kind::GT: rel = poly::SignCondition::GT; break;
                    case Kind::GEQ: rel = poly::SignCondition::GE; break;
                    default: Assert(false) << "Unexpected kind: " << k;
                }
                poly::Rational rhsmult;
                poly::Polynomial rhs = as_poly_polynomial(val, mMapper, rhsmult);
                rhsmult = poly::Rational(1) / rhsmult;
                // only correct up to a constant (denominator is thrown away!)
                //std::cout << "rhs = " << rhs << std::endl;
                if (!veq_c.isNull()) {
                    rhsmult = poly_utils::toRational(veq_c.getConst<Rational>());
                }
                mCandidates.emplace_back(Candidate{lhs, rel, rhs, rhsmult, n, collectVariables(val)});
                Trace("nl-icp") << "\tAdded " << mCandidates.back() << " from " << n << std::endl;
            } else if (res == -1) {
                poly::Variable lhs = mMapper(v);
                poly::SignCondition rel;
                switch (k) {
                    case Kind::LT: rel = poly::SignCondition::GT; break;
                    case Kind::LEQ: rel = poly::SignCondition::GE; break;
                    case Kind::EQUAL: rel = poly::SignCondition::EQ; break;
                    case Kind::DISTINCT: rel = poly::SignCondition::NE; break;
                    case Kind::GT: rel = poly::SignCondition::LT; break;
                    case Kind::GEQ: rel = poly::SignCondition::LE; break;
                    default: Assert(false) << "Unexpected kind: " << k;
                }
                poly::Rational rhsmult;
                poly::Polynomial rhs = as_poly_polynomial(val, mMapper, rhsmult);
                rhsmult = poly::Rational(1) / rhsmult;
                if (!veq_c.isNull()) {
                    rhsmult = poly_utils::toRational(veq_c.getConst<Rational>());
                }
                mCandidates.emplace_back(Candidate{lhs, rel, rhs, rhsmult, n, collectVariables(val)});
                Trace("nl-icp") << "\tAdded " << mCandidates.back() << " from " << n << std::endl;
            }
        }
    }

public:

    Propagator(): mBounds(mMapper) {}

    void add(const Node& n) {
        Trace("nl-icp") << "Trying to add " << n << std::endl;
        if (!mBounds.add(n)) {
            addCandidate(n);
        }
    }

    void init() {
        for (const auto& vars: mMapper.mVarCVCpoly) {
            auto& i = mBounds.get(vars.first);
            Trace("nl-icp") << "Adding initial " << vars.first << " -> " << i << std::endl;
            if (!i.lower_origin.isNull()) {
                Trace("nl-icp") << "\tAdding lower " << i.lower_origin << std::endl;
                mOrigins.add(vars.first, i.lower_origin, {});
            }
            if (!i.upper_origin.isNull()) {
                Trace("nl-icp") << "\tAdding upper " << i.upper_origin << std::endl;
                mOrigins.add(vars.first, i.upper_origin, {});
            }
        }
    }

    poly::IntervalAssignment getInitial() const {
        return mBounds.get();
    }

    PropagationResult doIt(poly::IntervalAssignment& ia) {
        if (mBudget <= 0) {
            return PropagationResult::NOT_CHANGED;
        }
        mLastConflict = Node();
        Trace("nl-icp") << "Starting propagation with " << IAWrapper{ia, mMapper} << std::endl;
        Trace("nl-icp") << "Current budget: " << mBudget << std::endl;
        PropagationResult res = PropagationResult::NOT_CHANGED;
        for (const auto& c: mCandidates) {
            --mBudget;
            PropagationResult cres = c.propagate(ia);
            switch (cres) {
                case PropagationResult::NOT_CHANGED:
                    break;
                case PropagationResult::CONTRACTED:
                case PropagationResult::CONTRACTED_STRONGLY:
                    mOrigins.add(mMapper(c.lhs), c.origin, c.rhsVariables);
                    res = PropagationResult::CONTRACTED;
                    break;
                case PropagationResult::CONTRACTED_WITHOUT_CURRENT:
                case PropagationResult::CONTRACTED_STRONGLY_WITHOUT_CURRENT:
                    mOrigins.add(mMapper(c.lhs), c.origin, c.rhsVariables, false);
                    res = PropagationResult::CONTRACTED;
                    break;
                case PropagationResult::CONFLICT:
                    mOrigins.add(mMapper(c.lhs), c.origin, c.rhsVariables);
                    auto nm = NodeManager::currentNM();
                    mLastConflict = nm->mkNode(Kind::NOT, nm->mkNode(Kind::AND, mOrigins.getOrigins(mMapper(c.lhs))));
                    return PropagationResult::CONFLICT;
            }
            switch (cres) {
                case PropagationResult::CONTRACTED_STRONGLY:
                case PropagationResult::CONTRACTED_STRONGLY_WITHOUT_CURRENT:
                    mBudget += mBudgetMultiplier;
                    break;
                default:
                    break;
            }
        }
        return res;
    }

    Node getConflict() const {
        return mLastConflict;
    }

    std::vector<Node> asLemmas(const poly::IntervalAssignment& ia) const {
        auto nm = NodeManager::currentNM();
        std::vector<Node> lemmas;

        for (const auto& vars: mMapper.mVarCVCpoly) {
            if (!ia.has(vars.second)) continue;
            Node v = vars.first;
            poly::Interval i = ia.get(vars.second);
            //std::cout << v << " -> " << i << std::endl;
            if (!is_minus_infinity(get_lower(i))) {
                Kind rel = get_lower_open(i) ? Kind::GT : Kind::GEQ;
                Node c = nm->mkNode(rel, v, value_to_node(get_lower(i), v));
                Node premise = nm->mkNode(Kind::AND, mOrigins.getOrigins(v));
                Trace("nl-icp") << premise << " => " << c << std::endl;
                Node lemma = Rewriter::rewrite(nm->mkNode(Kind::IMPLIES, premise, c));
                if (lemma.isConst()) {
                    Assert(lemma == nm->mkConst<bool>(true));
                } else {
                    Trace("nl-icp") << "Adding lemma " << lemma << std::endl;
                    lemmas.emplace_back(lemma);
                }
            }
            if (!is_plus_infinity(get_upper(i))) {
                Kind rel = get_upper_open(i) ? Kind::LT : Kind::LEQ;
                Node c = nm->mkNode(rel, v, value_to_node(get_upper(i), v));
                Node premise = nm->mkNode(Kind::AND, mOrigins.getOrigins(v));
                Trace("nl-icp") << premise << " => " << c << std::endl;
                Node lemma = Rewriter::rewrite(nm->mkNode(Kind::IMPLIES, premise, c));
                if (lemma.isConst()) {
                    Assert(lemma == nm->mkConst<bool>(true));
                } else {
                    Trace("nl-icp") << "Adding lemma " << lemma << std::endl;
                    lemmas.emplace_back(lemma);
                }
            }
        }
        return lemmas;
    }

    void print() {
        std::cout << mBounds << std::endl;
        std::cout << "Candidates:" << std::endl;
        for (const auto& c: mCandidates) {
            std::cout << "\t" << c << std::endl;
        }
        std::cout << mOrigins << std::endl;
    }
};

}
}
}
}
}

#endif