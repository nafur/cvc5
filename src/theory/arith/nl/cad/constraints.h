#ifndef CVC4__THEORY__NLARITH__CAD__CONSTRAINTS_H
#define CVC4__THEORY__NLARITH__CAD__CONSTRAINTS_H

#include "../libpoly/polynomial.h"
#include "../libpoly/sign_condition.h"
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

using namespace libpoly;

class Constraints
{
    using ConstraintVector = std::vector<std::pair<Polynomial, SignCondition>>;
  /** A list of constraints, each comprised of a polynomial and a relation symbol.
   */
  ConstraintVector mConstraints;

  /** A mapping from CVC4 variables to libpoly variables.
   */
  std::map<Node, Variable> mVariableMap;

  /** Get the corresponding variable or create a new one.
   */
  Variable get_variable(const Node& n)
  {
    Assert(n.getKind() == Kind::VARIABLE) << "Expect node to be a variable.";
    auto it = mVariableMap.find(n);
    if (it == mVariableMap.end())
    {
      std::string name;
      if (!n.getAttribute(expr::VarNameAttr(), name))
      {
        Trace("cad-check") << "Variable " << n
                           << " has no name, using ID instead." << std::endl;
        name = "v_" + std::to_string(n.getId());
      }
      it = mVariableMap.emplace(n, Variable(name.c_str())).first;
    }
    return it->second;
  }

  /** Checks whether the given relation can be handled by CAD.
   */
  bool is_suitable_relation(Kind kind) const
  {
    return (kind == Kind::EQUAL) || (kind == Kind::GT) || (kind == Kind::GEQ)
           || (kind == Kind::LT) || (kind == Kind::LEQ);
  }

  /** Normalizes two denominators.
   * Divides both by their gcd.
   */
  void normalize_denominators(Integer& d1, Integer& d2) const
  {
    Integer g = gcd(d1, d2);
    d1 /= g;
    d2 /= g;
  }

  /** Normalize the given kind, taking negation into account.
   * Always normalizes to EQUAL, DISTINCT, LT or LEQ and adapts the lhs accordingly.
   */
  SignCondition normalize_kind(Kind kind, bool negated, Polynomial& lhs) const
  {
    switch (kind)
    {
      case Kind::EQUAL:
      {
        return negated ? SignCondition::NE : SignCondition::EQ;
      }
      case Kind::LT:
      {
        if (negated)
        {
          lhs = -lhs;
          return SignCondition::LE;
        }
        return SignCondition::LT;
      }
      case Kind::LEQ:
      {
        if (negated)
        {
          lhs = -lhs;
          return SignCondition::LT;
        }
        return SignCondition::LE;
      }
      case Kind::GT:
      {
        if (negated)
        {
          return SignCondition::LE;
        }
        lhs = -lhs;
        return SignCondition::LT;
      }
      case Kind::GEQ:
      {
        if (negated)
        {
          return SignCondition::LT;
        }
        lhs = -lhs;
        return SignCondition::LE;
      }
      default:
        Assert(false) << "This function only deals with arithmetic relations.";
        return SignCondition::EQ;
    }
  }

  /** Constructs a polynomial from the given node.
   * 
   * While the Node n may contain rationals, Polynomial does not.
   * We therefore also store the denominator of the returned polynomial and
   * use it to construct the integer polynomial recursively.
   * Once the polynomial has been fully constructed, we can ignore the
   * denominator (except for its sign).
   */
  Polynomial construct_polynomial(const Node& n,
                                           Integer& denominator)
  {
    denominator = Integer(1);
    switch (n.getKind())
    {
      case Kind::VARIABLE:
      {
        return Polynomial(get_variable(n));
      }
      case Kind::CONST_RATIONAL:
      {
        Rational r = n.getConst<Rational>();
#if defined CVC4_GMP_IMP
        denominator = Integer(r.getDenominator().getValue());
        return Polynomial(
            Integer(r.getNumerator().getValue()));
#elif defined CVC4_CLN_IMP
        Assert(false) << "Did not implement number conversion for CLN yet";
#else
        Assert(false) << "Not sure which number type is used.";
#endif
        break;
      }
      case Kind::PLUS:
      {
        Polynomial res;
        Integer denom;
        for (const auto& child : n)
        {
          Polynomial tmp = construct_polynomial(child, denom);
          normalize_denominators(denom, denominator);
          res = res * denom + tmp * denominator;
          denominator *= denom;
        }
        return res;
      }
      case Kind::MULT:
      case Kind::NONLINEAR_MULT:
      {
        Polynomial res = Polynomial(denominator);
        Integer denom;
        for (const auto& child : n)
        {
          res *= construct_polynomial(child, denom);
          denominator *= denom;
        }
        return res;
      }
      default:
        Trace("cad-check") << "Unhandled node " << n << " with kind "
                           << n.getKind() << std::endl;
    }
    return Polynomial();
  }

  /** Constructs the lhs polynomial for a node representing a constraints.
   * Assumes the node to have exactly two children.
   */
  Polynomial construct_constraint_polynomial(const Node& n)
  {
    Assert(n.getNumChildren() == 2)
        << "Supported relations only have two children.";
    auto childit = n.begin();
    Integer ldenom;
    Polynomial left = construct_polynomial(*childit++, ldenom);
    Integer rdenom;
    Polynomial right = construct_polynomial(*childit++, rdenom);
    Assert(childit == n.end()) << "Screwed up iterator handling.";

    normalize_denominators(ldenom, rdenom);
    return left * rdenom - right * ldenom;
  }

 public:
    void add_constraint(const Polynomial& lhs, SignCondition sc) {
        mConstraints.emplace_back(lhs, sc);
    }
  /** Add a constraints (represented by a node) to the list of constraints.
   */
  void add_constraint(Node n)
  {
    bool negated = false;
    if (n.getKind() == Kind::NOT)
    {
      Assert(n.getNumChildren() == 1)
          << "Expect negations to have a single child.";
      negated = true;
      n = *n.begin();
    }
    Assert(is_suitable_relation(n.getKind()))
        << "Found a constraint with unsupported relation " << n.getKind();

    Polynomial lhs = construct_constraint_polynomial(n);
    SignCondition sc = normalize_kind(n.getKind(), negated, lhs);
    Trace("cad-check") << "Parsed " << lhs << " " << sc << " 0"
                       << std::endl;
    add_constraint(lhs, sc);
  }

  const ConstraintVector& get_constraints() const {
      return mConstraints;
  }
};

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif