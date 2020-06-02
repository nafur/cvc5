
#ifndef CVC4__THEORY__NLARITH__CAD__CONSTRAINTS_H
#define CVC4__THEORY__NLARITH__CAD__CONSTRAINTS_H

#include "../libpoly/polynomial.h"
#include "expr/kind.h"
#include "expr/node_manager_attributes.h"

#include <iostream>
#include <map>
#include <vector>

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

class Constraints {
    std::vector<std::pair<libpoly::Polynomial, CVC4::Kind>> mConstraints;

    std::map<Node,libpoly::Variable> mVariableMap;

    libpoly::Variable get_variable(const Node& n) {
        Assert(n.getKind() == Kind::VARIABLE) << "Expect node to be a variable.";
        auto it = mVariableMap.find(n);
        if (it == mVariableMap.end()) {
            std::string name;
            if (!n.getAttribute(expr::VarNameAttr(), name)) {
                Trace("cad-check") << "Variable " << n << " has no name, using ID instead." << std::endl;
                name = "v_" + std::to_string(n.getId());
            }
            it = mVariableMap.emplace(n, libpoly::Variable(name.c_str())).first;
        }
        return it->second;
    }

    bool is_suitable_relation(Kind kind) const {
        return (kind == Kind::EQUAL) ||
            (kind == Kind::GT) || (kind == Kind::GEQ) ||
            (kind == Kind::LT) || (kind == Kind::LEQ);
    }

    void normalize_denominators(libpoly::Integer& d1, libpoly::Integer& d2) const {
        libpoly::Integer g = gcd(d1, d2);
        d1 /= g;
        d2 /= g;
    }

    Kind normalize_kind(Kind kind, bool negated, libpoly::Polynomial& lhs) const {
        switch (kind) {
            case Kind::EQUAL: {
                return negated ? Kind::DISTINCT : Kind::EQUAL;
            }
            case Kind::LT: {
                if (negated) {
                    lhs = -lhs;
                    return Kind::LEQ;
                }
                return Kind::LT;
            }
            case Kind::LEQ: {
                if (negated) {
                    lhs = -lhs;
                    return Kind::LT;
                }
                return Kind::LEQ;
            }
            case Kind::GT: {
                if (negated) {
                    return Kind::LEQ;
                }
                lhs = -lhs;
                return Kind::LT;
            }
            case Kind::GEQ: {
                if (negated) {
                    return Kind::LT;
                }
                lhs = -lhs;
                return Kind::LEQ;
            }
            default:
                Assert(false) << "This function only deals with arithmetic relations.";
                return Kind::EQUAL;
        }
    }
    
    /* While the Node n may contain rationals, libpoly::Polynomial does not.
     * We therefore also store the denominator of the returned polynomial and
     * use it to construct the integer polynomial recursively.
     * Once the polynomial has been fully constructed, we can ignore the denominator (except for its sign).
     */
    libpoly::Polynomial construct_polynomial(const Node& n, libpoly::Integer& denominator) {
        denominator = libpoly::Integer(1);
        switch (n.getKind()) {
            case Kind::VARIABLE: {
                return libpoly::Polynomial(get_variable(n));
            }
            case Kind::CONST_RATIONAL: {
                Rational r = n.getConst<Rational>();
                #ifdef CVC4_GMP_IMP
                denominator = libpoly::Integer(r.getDenominator().getValue());
                return libpoly::Polynomial(libpoly::Integer(r.getNumerator().getValue()));
                #elif CVC4_CLN_IMP
                Assert(false) << "Did not implement number conversion for CLN yet";
                #else
                Assert(false) << "Not sure which number type is used.";
                #endif
                break;
            }
            case Kind::PLUS: {
                libpoly::Polynomial res;
                libpoly::Integer denom;
                for (const auto& child: n) {
                    libpoly::Polynomial tmp = construct_polynomial(child, denom);
                    normalize_denominators(denom, denominator);
                    res = res * denom + tmp * denominator;
                    denominator *= denom;
                }
                return res;
            }
            case Kind::MULT:
            case Kind::NONLINEAR_MULT: {
                libpoly::Polynomial res = libpoly::Polynomial(denominator);
                libpoly::Integer denom;
                for (const auto& child: n) {
                    res *= construct_polynomial(child, denom);
                    denominator *= denom;
                }
                return res;
            }
            default:
                Trace("cad-check") << "Unhandled node " << n << " with kind " << n.getKind() << std::endl;
        }
        return libpoly::Polynomial();
    }
    libpoly::Polynomial construct_constraint_polynomial(const Node& n) {
        Assert(n.getNumChildren() == 2) << "Supported relations only have two children.";
        auto childit = n.begin();
        libpoly::Integer ldenom;
        libpoly::Polynomial left = construct_polynomial(*childit++, ldenom);
        libpoly::Integer rdenom;
        libpoly::Polynomial right = construct_polynomial(*childit++, rdenom);
        Assert(childit == n.end()) << "Screwed up iterator handling.";
        
        normalize_denominators(ldenom, rdenom);
        return left * rdenom - right * ldenom;
    }
public:
    void add_constraint(Node n) {
        bool negated = false;
        if (n.getKind() == Kind::NOT) {
            Assert(n.getNumChildren() == 1) << "Expect negations to have a single child.";
            negated = true;
            n = *n.begin();
        }
        Assert(is_suitable_relation(n.getKind())) << "Found a constraint with unsupported relation " << n.getKind();

        libpoly::Polynomial lhs = construct_constraint_polynomial(n);
        Kind relation = normalize_kind(n.getKind(), negated, lhs);
        Trace("cad-check") << "Parsed " << lhs << " " << relation << " 0" << std::endl;

        mConstraints.emplace_back(lhs, relation);
    }
};

}
}
}
}
}

#endif