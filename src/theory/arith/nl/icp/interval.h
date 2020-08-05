#ifndef CVC4__THEORY__ARITH__ICP__INTERVAL_H
#define CVC4__THEORY__ARITH__ICP__INTERVAL_H

#include "expr/node.h"
#include "expr/node_algorithm.h"
#include "theory/arith/arith_msum.h"
#include "theory/arith/normal_form.h"

#include <poly/polyxx.h>
#include "util/poly_util.h"
#include "theory/arith/nl/poly_conversion.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace icp {

struct Interval {
    poly::Value lower = poly::Value::minus_infty();
    bool lower_strict = true;
    Node lower_origin;
    poly::Value upper = poly::Value::plus_infty();
    bool upper_strict = true;
    Node upper_origin;
};

inline std::ostream& operator<<(std::ostream& os, const Interval& i) {
    return os << (i.lower_strict ? '(' : '[') << i.lower << " .. " << i.upper << (i.upper_strict ? ')' : ']');
}

class VariableBounds {
    VariableMapper& mMapper;
    std::map<Node,Interval> mIntervals;

    void update_lower_bound(const Node& origin, const Node& variable, const poly::Value& value, bool strict) {
        // variable > or >= value because of origin
        Trace("nl-icp") << "\tNew bound " << variable << (strict ? ">" : ">=") << value << " due to " << origin << std::endl;
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
        Trace("nl-icp") << "\tNew bound " << variable << (strict ? "<" : "<=") << value << " due to " << origin << std::endl;
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
public:
    VariableBounds(VariableMapper& mapper): mMapper(mapper) {}

    const VariableMapper& getMapper() const {
        return mMapper;
    }

    Interval& get(const Variable& v) {
        return get(v.getNode());
    }
    Interval& get(const Node& v) {
        auto it = mIntervals.find(v);
        if (it == mIntervals.end()) {
            it = mIntervals.emplace(v, Interval()).first;
        }
        return it->second;
    }
    Interval get(const Variable& v) const {
        auto it = mIntervals.find(v.getNode());
        if (it == mIntervals.end()) {
            return Interval{};
        }
        return it->second;
    }

    std::vector<Node> getOrigins() const {
        std::vector<Node> res;
        for (const auto& vi: mIntervals) {
            if (!vi.second.lower_origin.isNull()) {
                res.emplace_back(vi.second.lower_origin);
            }
            if (!vi.second.upper_origin.isNull()) {
                if (!res.empty() && vi.second.upper_origin != res.back()) {
                    res.emplace_back(vi.second.upper_origin);
                }
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
            mMapper(v.getNode());
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
};

inline std::ostream& operator<<(std::ostream& os, const VariableBounds& vb) {
    os << "Bounds:" << std::endl;
    for (const auto& var: vb.getMapper().mVarCVCpoly) {
        os << "\t" << var.first << " -> " << vb.get(var.first) << std::endl;;
    }
    return os;
}

enum class PropagationResult {
    NOT_CHANGED,
    CONTRACTED,
    CONTRACTED_WITHOUT_CURRENT,
    CONFLICT
};

PropagationResult intersect_interval_with(poly::Interval& cur, const poly::Interval& res) {
    Trace("nl-icp") << "Updating " << cur << " with " << res << std::endl;

    constexpr std::size_t size_threshold = 100;
    if (bitsize(get_lower(res)) > size_threshold || bitsize(get_upper(res)) > size_threshold) {
        Trace("nl-icp") << "Reached bitsize threshold" << std::endl;
        return PropagationResult::NOT_CHANGED;
    }

    // bounds for res have 5 positions:
    // 1 < 2 (lower(cur)) < 3 < 4 (upper(cur)) < 5

    if (get_upper(res) < get_lower(cur)) {
        // upper(res) at 1
        Trace("nl-icp") << "res < cur -> conflict" << std::endl;
        return PropagationResult::CONFLICT;
    }
    if (get_upper(res) == get_lower(cur)) {
        // upper(res) at 2
        if (get_upper_open(res) || get_lower_open(cur)) {
            Trace("nl-icp") << "meet at lower, but one is open -> conflict" << std::endl;
            return PropagationResult::CONFLICT;
        }
        if (!is_point(cur)) {
            Trace("nl-icp") << "contracts to point interval at lower" << std::endl;
            cur = poly::Interval(get_upper(res));
            return PropagationResult::CONTRACTED;
        }
        return PropagationResult::NOT_CHANGED;
    }
    Assert(get_upper(res) > get_lower(cur)) << "Comparison operator does weird stuff.";
    if (get_upper(res) < get_upper(cur)) {
        // upper(res) at 3
        if (get_lower(res) < get_lower(cur)) {
            // lower(res) at 1
            Trace("nl-icp") << "lower(cur) .. upper(res)" << std::endl;
            cur.set_upper(get_upper(res), get_upper_open(res));
            return PropagationResult::CONTRACTED;
        }
        if (get_lower(res) == get_lower(cur)) {
            // lower(res) at 2
            Trace("nl-icp") << "meet at lower, lower(cur) .. upper(res)" << std::endl;
            cur = poly::Interval(
                get_lower(cur), get_lower_open(cur) || get_lower_open(res), get_upper(res), get_upper_open(res)
            );
            if (get_lower_open(cur) && !get_lower_open(res)) {
                return PropagationResult::CONTRACTED;
            } else {
                return PropagationResult::CONTRACTED_WITHOUT_CURRENT;
            }
        }
        Assert(get_lower(res) > get_lower(cur)) << "Comparison operator does weird stuff.";
        // lower(res) at 3
        Trace("nl-icp") << "cur covers res" << std::endl;
        cur = res;
        return PropagationResult::CONTRACTED_WITHOUT_CURRENT;
    }
    if (get_upper(res) == get_upper(cur)) {
        // upper(res) at 4
        if (get_lower(res) < get_lower(cur)) {
            // lower(res) at 1
            Trace("nl-icp") << "res covers cur but meet at upper" << std::endl;
            if (get_upper_open(res) && !get_upper_open(cur)) {
                cur.set_upper(get_upper(cur), true);
                return PropagationResult::CONTRACTED;
            }
            return PropagationResult::NOT_CHANGED;
        }
        if (get_lower(res) == get_lower(cur)) {
            // lower(res) at 2
            Trace("nl-icp") << "same bounds but check openness" << std::endl;
            bool changed = false;
            if (get_lower_open(res) && !get_lower_open(cur)) {
                changed = true;
                cur.set_lower(get_lower(cur), true);
            }
            if (get_upper_open(res) && !get_upper_open(cur)) {
                changed = true;
                cur.set_upper(get_upper(cur), true);
            }
            if (changed) {
                if (
                       (get_lower_open(res) || !get_upper_open(cur))
                    && (get_upper_open(res) || !get_upper_open(cur))
                ) {
                    return PropagationResult::CONTRACTED_WITHOUT_CURRENT;
                } else {
                    return PropagationResult::CONTRACTED;
                }
            }
            return PropagationResult::NOT_CHANGED;
        }
        Assert(get_lower(res) > get_lower(cur)) << "Comparison operator does weird stuff.";
        // lower(res) at 3
        Trace("nl-icp") << "cur covers res but meet at upper" << std::endl;
        cur = poly::Interval(
            get_lower(res), get_lower_open(res), get_upper(res), get_upper_open(cur) || get_upper_open(res)
        );
        if (get_upper_open(cur) && !get_upper_open(res)) {
            return PropagationResult::CONTRACTED;
        } else {
            return PropagationResult::CONTRACTED_WITHOUT_CURRENT;
        }
    }

    Assert(get_upper(res) > get_upper(cur)) << "Comparison operator does weird stuff.";
    // upper(res) at 5
    
    if (get_lower(res) < get_lower(cur)) {
        // lower(res) at 1
        Trace("nl-icp") << "res covers cur" << std::endl;
        return PropagationResult::NOT_CHANGED;
    }
    if (get_lower(res) == get_lower(cur)) {
        // lower(res) at 2
        Trace("nl-icp") << "res covers cur but meet at lower" << std::endl;
        if (get_lower_open(res) && is_point(cur)) {
            return PropagationResult::CONFLICT;
        }
        else if (get_lower_open(res) && !get_lower_open(cur)) {
            cur.set_lower(get_lower(cur), true);
            return PropagationResult::CONTRACTED;
        }
        return PropagationResult::NOT_CHANGED;
    }
    Assert(get_lower(res) > get_lower(cur)) << "Comparison operator does weird stuff.";
    if (get_lower(res) < get_upper(cur)) {
        // lower(res) at 3
        Trace("nl-icp") << "lower(res) .. upper(cur)" << std::endl;
        cur.set_lower(get_lower(res), get_lower_open(res));
        return PropagationResult::CONTRACTED;
    }
    if (get_lower(res) == get_upper(cur)) {
        // lower(res) at 4
        if (get_lower_open(res) || get_upper_open(cur)) {
            Trace("nl-icp") << "meet at upper, but one is open -> conflict" << std::endl;
            return PropagationResult::CONFLICT;
        }
        if (!is_point(cur)) {
            Trace("nl-icp") << "contracts to point interval at upper" << std::endl;
            cur = poly::Interval(get_lower(res));
            return PropagationResult::CONTRACTED;
        }
        return PropagationResult::NOT_CHANGED;
    }

    Assert(get_lower(res) > get_upper(cur));
    // lower(res) at 5
    Trace("nl-icp") << "res > cur -> conflict" << std::endl;
    return PropagationResult::CONFLICT;
}

struct Candidate {
    poly::Variable lhs;
    poly::SignCondition rel;
    poly::Polynomial rhs;
    poly::Rational rhsmult;
    Node origin;
    std::vector<Node> rhsVariables;

    PropagationResult propagate(poly::IntervalAssignment& ia) const {
        auto res = poly::evaluate(rhs, ia) * poly::Interval(poly::Value(rhsmult));
        Trace("nl-icp") << "Prop: " << *this << " -> " << res << std::endl;
        switch (rel) {
            case poly::SignCondition::LT:
                res.set_lower(poly::Value::minus_infty(), true);
                res.set_upper(get_upper(res), true);
                break;
            case poly::SignCondition::LE: res.set_lower(poly::Value::minus_infty(), true); break;
            case poly::SignCondition::EQ: break;
            case poly::SignCondition::NE: Assert(false); break;
            case poly::SignCondition::GT:
                res.set_lower(get_lower(res), true);
                res.set_upper(poly::Value::plus_infty(), true);
                break;
            case poly::SignCondition::GE: res.set_upper(poly::Value::plus_infty(), true); break;
        }
        auto cur = ia.get(lhs);
        Trace("nl-icp") << "-> " << res << " used to update " << cur << std::endl;

        PropagationResult result = intersect_interval_with(cur, res);
        if (result == PropagationResult::CONTRACTED || result == PropagationResult::CONTRACTED_WITHOUT_CURRENT) {
            Trace("nl-icp") << *this << " contracted " << lhs << " -> " << cur << std::endl;
            ia.set(lhs, cur);
        }
        return result;
    }
};
inline std::ostream& operator<<(std::ostream& os, const Candidate& c) {
    os << c.lhs << " " << c.rel << " ";
    if (c.rhsmult != poly::Rational(1)) os << c.rhsmult << " * ";
    return os << c.rhs;
}

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

class ContractionOriginManager {
    struct ContractionOrigin {
        Node candidate;
        std::vector<ContractionOrigin*> origins;
    };
    friend std::ostream& operator<<(std::ostream& os, const ContractionOriginManager& com);
    friend void print(std::ostream& os, const std::string& indent, const ContractionOrigin* co);

    void getOrigins(ContractionOrigin const * const origin, std::set<Node>& res) const {
        if (!origin->candidate.isNull()) {
            res.insert(origin->candidate);
        }
        for (const auto& co: origin->origins) {
            getOrigins(co, res);
        }
    }
public:
    void add(const Node& targetVariable, const Node& candidate, const std::vector<Node>& originVariables, bool addTarget = true) {
        Trace("nl-icp") << "Adding contraction for " << targetVariable << std::endl;
        std::vector<ContractionOrigin*> origins;
        if (addTarget) {
            auto it = d_currentOrigins.find(targetVariable);
            if (it != d_currentOrigins.end()) {
                origins.emplace_back(it->second);
            }
        }
        for (const auto& v: originVariables) {
            auto it = d_currentOrigins.find(v);
            if (it != d_currentOrigins.end()) {
                origins.emplace_back(it->second);
            }
        }
        d_allocations.emplace_back(new ContractionOrigin{candidate, std::move(origins)});
        d_currentOrigins[targetVariable] = d_allocations.back().get();
    }

    std::vector<Node> getOrigins(const Node& variable) const {
        Trace("nl-icp") << "Obtaining origins for " << variable << std::endl;
        std::set<Node> origins;
        Assert(d_currentOrigins.find(variable) != d_currentOrigins.end()) << "Using variable as origin that is unknown yet.";
        getOrigins(d_currentOrigins.at(variable), origins);
        return std::vector<Node>(origins.begin(), origins.end());
    }
private:
    std::map<Node,ContractionOrigin*> d_currentOrigins;
    std::vector<std::unique_ptr<ContractionOrigin>> d_allocations;
};

inline void print(std::ostream& os, const std::string& indent, const ContractionOriginManager::ContractionOrigin* co) {
    if (!co->candidate.isNull()) {
        os << indent << co->candidate << std::endl;
    } else {
        os << indent << "<null>" << std::endl;
    }
    for (const auto& o: co->origins) {
        print(os, indent + "\t", o);
    }
}

inline std::ostream& operator<<(std::ostream& os, const ContractionOriginManager& com) {
    os << "ContractionOrigins:" << std::endl;
    for (const auto& vars: com.d_currentOrigins) {
        os << vars.first << ":" << std::endl;
        print(os, "\t", vars.second);
    }
    return os;
}

class Propagator {
    VariableMapper mMapper;
    VariableBounds mBounds;
    std::vector<Candidate> mCandidates;
    ContractionOriginManager mOrigins;
    Node mLastConflict;

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
        mLastConflict = Node();
        Trace("nl-icp") << "Starting propagation with " << IAWrapper{ia, mMapper} << std::endl;
        PropagationResult res = PropagationResult::NOT_CHANGED;
        for (const auto& c: mCandidates) {
            PropagationResult cres = c.propagate(ia);
            switch (cres) {
                case PropagationResult::NOT_CHANGED:
                    break;
                case PropagationResult::CONTRACTED:
                    mOrigins.add(mMapper(c.lhs), c.origin, c.rhsVariables);
                    res = PropagationResult::CONTRACTED;
                    break;
                case PropagationResult::CONTRACTED_WITHOUT_CURRENT:
                    mOrigins.add(mMapper(c.lhs), c.origin, c.rhsVariables, false);
                    res = PropagationResult::CONTRACTED;
                    break;
                case PropagationResult::CONFLICT:
                    mOrigins.add(mMapper(c.lhs), c.origin, c.rhsVariables);
                    auto nm = NodeManager::currentNM();
                    mLastConflict = nm->mkNode(Kind::NOT, nm->mkNode(Kind::AND, mOrigins.getOrigins(mMapper(c.lhs))));
                    return PropagationResult::CONFLICT;
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