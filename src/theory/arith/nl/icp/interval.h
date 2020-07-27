#ifndef CVC4__THEORY__ARITH__ICP__INTERVAL_H
#define CVC4__THEORY__ARITH__ICP__INTERVAL_H

#include "expr/node.h"
#include "expr/node_algorithm.h"
#include "theory/arith/arith_msum.h"
#include "theory/arith/normal_form.h"

#include <poly/polyxx.h>
#include "theory/arith/nl/poly_conversion.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace icp {

class VariableBounds {
    VariableMapper& mMapper;
    struct Interval {
        poly::Value lower = poly::Value::minus_infty();
        bool lower_strict = true;
        Node lower_origin;
        poly::Value upper = poly::Value::plus_infty();
        bool upper_strict = true;
        Node upper_origin;
    };
    std::map<Node,Interval> mIntervals;

    void update_lower_bound(const Node& origin, const Node& variable, const poly::Value& value, bool strict) {
        // variable > or >= value because of origin
        Interval& i = get(variable);
        if (poly::is_none(i.lower) || i.lower < value) {
            i.lower = value;
            i.lower_strict = strict;
            i.lower_origin = origin;
        } else if (strict && i.lower == value) {
            i.lower_strict = strict;
            i.lower_origin = origin;
        }
    }
    void update_upper_bound(const Node& origin, const Node& variable, const poly::Value& value, bool strict) {
        // variable < or <= value because of origin
        Interval& i = get(variable);
        if (poly::is_none(i.upper) || i.upper > value) {
            i.upper = value;
            i.upper_strict = strict;
            i.upper_origin = origin;
        } else if (strict && i.upper == value) {
            i.upper_strict = strict;
            i.upper_origin = origin;
        }
    }
    Interval& get(const Variable& v) {
        auto it = mIntervals.find(v.getNode());
        if (it == mIntervals.end()) {
            it = mIntervals.emplace(v.getNode(), Interval()).first;
        }
        return it->second;
    }
public:
    VariableBounds(VariableMapper& mapper): mMapper(mapper) {}

    std::vector<Node> getOrigins() const {
        std::vector<Node> res;
        for (const auto& vi: mIntervals) {
            if (!vi.second.lower_origin.isNull()) {
                res.emplace_back(vi.second.lower_origin);
            }
            if (!vi.second.upper_origin.isNull()) {
                res.emplace_back(vi.second.upper_origin);
            }
        }
        return res;
    }

    poly::IntervalAssignment get() const {
        poly::IntervalAssignment res;
        for (const auto& vi: mIntervals) {
            poly::Variable v = mMapper(vi.first);
            poly::Interval i(vi.second.lower, vi.second.lower_strict, vi.second.upper, vi.second.upper_strict);
            res.set(v, i);
        }
        return res;
    }
    bool add(const Node& n) {
        Trace("nl-icp") << "Add bound " << n << std::endl;
        auto comp = Comparison::parseNormalForm(n);
        auto foo = comp.decompose(true);
        if (std::get<0>(foo).isVariable()) {
            Variable v = std::get<0>(foo).getVariable();
            Kind relation = std::get<1>(foo);
            if (relation == Kind::DISTINCT) return false;
            Constant bound = std::get<2>(foo);

            //std::cout << n << std::endl;
            //std::cout << "-> " << v.getNode() << " " << relation << " " << bound.getNode() << std::endl;

            poly::Value val = node_to_value(bound.getNode(), v.getNode());
            poly::Interval newi = poly::Interval::full();

            Maybe<Node> res;
            switch (relation) {
                case Kind::LEQ: update_upper_bound(n, v.getNode(), val, false); break;
                case Kind::LT: update_upper_bound(n, v.getNode(), val, true); break;
                case Kind::EQUAL:
                    update_lower_bound(n, v.getNode(), val, false);
                    update_upper_bound(n, v.getNode(), val, false);
                    break;
                case Kind::GT: update_lower_bound(n, v.getNode(), val, true); break;
                case Kind::GEQ: update_lower_bound(n, v.getNode(), val, false); break;
                default:
                    Assert(false);
            }
            return true;
        }
        return false;
    }

    Maybe<std::pair<Node,Node>> hasConflict() const {
        for (const auto& vi: mIntervals) {
            const Interval& i = vi.second;
            if (i.lower > i.upper) {
                return std::pair<Node,Node>(i.lower_origin, i.upper_origin);
            }
            if ((i.lower_strict || i.upper_strict) && i.lower == i.upper) {
                return std::pair<Node,Node>(i.lower_origin, i.upper_origin);
            }
        }
        return Maybe<std::pair<Node,Node>>();
    }

    void print() {
        std::cout << "Intervals:" << std::endl;
        for (const auto& i: mIntervals) {
            std::cout << "\t" << i.first << " -> " << i.second.lower << " .. " << i.second.upper << std::endl;
        }
    }
};

inline std::ostream& operator<<(std::ostream& os, const VariableBounds& vb) {
    return os << "Bounds: " << std::endl << vb.get() << std::endl;
}

struct Candidate {
    poly::Variable lhs;
    poly::SignCondition rel;
    poly::Polynomial rhs;
    poly::Rational rhsmult;
    Node origin;

    bool propagate(poly::IntervalAssignment& ia) const {
        auto res = poly::evaluate(rhs, ia) * poly::Interval(poly::Value(rhsmult));
        switch (rel) {
            case poly::SignCondition::LT: res.set_lower(poly::Value::minus_infty(), true); break;
            case poly::SignCondition::LE: res.set_lower(poly::Value::minus_infty(), true); break;
            case poly::SignCondition::EQ: break;
            case poly::SignCondition::NE: Assert(false); break;
            case poly::SignCondition::GT: res.set_upper(poly::Value::plus_infty(), true); break;
            case poly::SignCondition::GE: res.set_upper(poly::Value::plus_infty(), true); break;
        }
        auto cur = ia.get(lhs);
        bool changed = false;
        if (get_lower(res) > get_lower(cur)) {
            static const poly::Value min_threshold = poly::Value(poly::Integer(-1000000));
            if (get_lower(cur) > min_threshold) {
                cur.set_lower(get_lower(res), false);
                changed = true;
            }
        }
        if (get_upper(res) < get_upper(cur)) {
            static const poly::Value max_threshold = poly::Value(poly::Integer(1000000));
            if (get_upper(cur) < max_threshold) {
                cur.set_upper(get_upper(res), false);
                changed = true;
            }
        }
        if (changed) {
            Trace("nl-icp") << *this << " propagated " << lhs << " -> " << cur << std::endl;
            ia.set(lhs, cur);
        }
        return changed;
    }
};
inline std::ostream& operator<<(std::ostream& os, const Candidate& c) {
    os << c.lhs << " " << c.rel << " ";
    if (c.rhsmult != poly::Rational(1)) os << c.rhsmult << " * ";
    return os << c.rhs;
}

class Propagator {
    VariableMapper mMapper;
    VariableBounds mBounds;
    std::vector<Candidate> mCandidates;
    std::vector<Node> mLastConflict;
    std::set<Node> mUsedCandidates;

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
            //std::cout << "Checking " << n << " for " << v << std::endl;

            std::map<Node, Node> msum;
            ArithMSum::getMonomialSum(poly.getNode(), msum);

            Node veq_c;
            Node val;
            
            int res = ArithMSum::isolate(v, msum, veq_c, val, k);
            //std::cout << "Isolate: " << veq_c << " and " << val << std::endl;
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
                    
                mCandidates.emplace_back(Candidate{lhs, rel, rhs, rhsmult, n});
                Trace("nl-icp") << "Added " << mCandidates.back() << " from " << n << std::endl;
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
                mCandidates.emplace_back(Candidate{lhs, rel, rhs, rhsmult, n});
                Trace("nl-icp") << "Added " << mCandidates.back() << " from " << n << std::endl;
            }
        }
    }

public:

    Propagator(): mBounds(mMapper) {}

    void add(const Node& n) {
        if (!mBounds.add(n)) {
            addCandidate(n);
        }
    }

    poly::IntervalAssignment getInitial() const {
        return mBounds.get();
    }

    bool doIt(poly::IntervalAssignment& ia) {
        Trace("nl-icp") << "Starting propagation with " << ia << std::endl;
        bool propagated = false;
        for (const auto& c: mCandidates) {
            if (c.propagate(ia)) {
                propagated = true;
                mUsedCandidates.insert(c.origin);
            }
        }
        return propagated;
    }

    std::vector<Node> asLemmas(const poly::IntervalAssignment& ia) const {
        auto nm = NodeManager::currentNM();
        std::vector<Node> premises = mBounds.getOrigins();
        premises.insert(premises.end(), mUsedCandidates.begin(), mUsedCandidates.end());
        Node premise = nm->mkNode(Kind::AND, premises);
        std::vector<Node> conclusions;


        for (const auto& vars: mMapper.mVarCVCpoly) {
            if (!ia.has(vars.second)) continue;
            Node v = vars.first;
            poly::Interval i = ia.get(vars.second);
            //std::cout << v << " -> " << i << std::endl;
            if (!is_minus_infinity(get_lower(i))) {
                Kind rel = get_lower_open(i) ? Kind::GT : Kind::GEQ;
                Node c = nm->mkNode(rel, v, value_to_node(get_lower(i), v));
                conclusions.emplace_back(c);
            }
            if (!is_plus_infinity(get_upper(i))) {
                Kind rel = get_upper_open(i) ? Kind::LT : Kind::LEQ;
                Node c = nm->mkNode(rel, v, value_to_node(get_upper(i), v));
                conclusions.emplace_back(c);
            }
        }

        std::vector<Node> lemmas;
        for (const auto& c: conclusions) {
            Node lemma = nm->mkNode(Kind::IMPLIES, premise, c);
            Node rewritten = Rewriter::rewrite(lemma);
            if (rewritten.isConst()) {
                Assert(rewritten == nm->mkConst<bool>(true));
            } else {
                Trace("nl-icp") << "Adding lemma " << lemma << std::endl;
                lemmas.emplace_back(rewritten);
            }
        }
        
        return lemmas;
    }

    void print() {
        mBounds.print();
        std::cout << "Candidates:" << std::endl;
        for (const auto& c: mCandidates) {
            std::cout << "\t" << c << std::endl;
        }
    }
};

}
}
}
}
}

#endif