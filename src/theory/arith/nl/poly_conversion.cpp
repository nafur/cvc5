#include "poly_conversion.h"

#include "expr/node.h"
#include "expr/node_manager_attributes.h"
#include "util/integer.h"
#include "util/rational.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {

libpoly::Variable VariableMapper::operator()(const CVC4::Node& n)
{
  Assert(n.isVar()) << "Expect node to be a variable.";
  auto it = mVarCVCpoly.find(n);
  if (it == mVarCVCpoly.end())
  {
    std::string name;
    if (!n.getAttribute(expr::VarNameAttr(), name))
    {
      Trace("libpoly::conversion")
          << "Variable " << n << " has no name, using ID instead." << std::endl;
      name = "v_" + std::to_string(n.getId());
    }
    it = mVarCVCpoly.emplace(n, Variable(name.c_str())).first;
    mVarpolyCVC.emplace(it->second, n);
  }
  return it->second;
}

CVC4::Node VariableMapper::operator()(const libpoly::Variable& n)
{
  auto it = mVarpolyCVC.find(n);
  Assert(it != mVarpolyCVC.end()) << "Expect variable to be added already.";
  return it->second;
}

CVC4::Integer as_cvc_integer(const lp_integer_t* v)
{
  // TODO(Gereon): Make this conversion more efficient.
  std::string tmp(lp_integer_to_string(v));
  return CVC4::Integer(tmp);
}
CVC4::Rational as_cvc_rational(const lp_rational_t* v)
{
  // TODO(Gereon): Make this conversion more efficient.
  std::string tmp(lp_rational_to_string(v));
  return CVC4::Rational(tmp);
}
CVC4::Rational as_cvc_rational(const lp_dyadic_rational_t* v)
{
  // TODO(Gereon): Make this conversion more efficient.
  std::string tmp(lp_dyadic_rational_to_string(v));
  return CVC4::Rational(tmp);
}

CVC4::Node as_cvc_polynomial(const UPolynomial& p, const CVC4::Node& var)
{
  Trace("libpoly::conversion")
      << "Converting " << p << " over " << var << std::endl;

  std::size_t size = degree(p) + 1;
  lp_integer_t coeffs[size];
  for (std::size_t i = 0; i < size; ++i)
  {
    lp_integer_construct_from_int(lp_Z, &coeffs[i], 0);
  }

  lp_upolynomial_unpack(p.get(), coeffs);
  auto* nm = NodeManager::currentNM();

  Node res = nm->mkConst(Rational(0));
  Node monomial = nm->mkConst(Rational(1));
  for (std::size_t i = 0; i < size; ++i)
  {
    if (!lp_integer_is_zero(lp_Z, &coeffs[i]))
    {
      Node coeff = nm->mkConst(CVC4::Rational(as_cvc_integer(&coeffs[i])));
      Node term = nm->mkNode(Kind::MULT, coeff, monomial);
      res = nm->mkNode(Kind::PLUS, res, term);
    }
    monomial = nm->mkNode(Kind::NONLINEAR_MULT, monomial, var);
    lp_integer_destruct(&coeffs[i]);
  }
  Trace("libpoly::conversion") << "-> " << res << std::endl;
  return res;
}


Polynomial as_poly_polynomial_impl(const CVC4::Node& n,
                                   Integer& denominator,
                                   VariableMapper& vm)
{
  denominator = Integer(1);
  if (n.isVar()) {
    return Polynomial(vm(n));
  }
  switch (n.getKind())
  {
    case Kind::CONST_RATIONAL:
    {
      Rational r = n.getConst<Rational>();
      denominator = Integer(r.getDenominator().getValue());
      return Polynomial(Integer(r.getNumerator().getValue()));
    }
    case Kind::PLUS:
    {
      Polynomial res;
      Integer denom;
      for (const auto& child : n)
      {
        Polynomial tmp = as_poly_polynomial_impl(child, denom, vm);
        /** Normalize denominators
         */
        Integer g = gcd(denom, denominator);
        res = res * (denom/g) + tmp * (denominator/g);
        denominator *= (denom/g);
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
        res *= as_poly_polynomial_impl(child, denom, vm);
        denominator *= denom;
      }
      return res;
    }
    default:
      Warning() << "Unhandled node " << n << " with kind " << n.getKind()
                << std::endl;
  }
  return Polynomial();
}
Polynomial as_poly_polynomial(const CVC4::Node& n, VariableMapper& vm)
{
  Integer denom;
  return as_poly_polynomial_impl(n, denom, vm);
}

libpoly::SignCondition normalize_kind(CVC4::Kind kind,
                                      bool negated,
                                      Polynomial& lhs)
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

std::pair<libpoly::Polynomial, libpoly::SignCondition> as_poly_constraint(
    Node n, VariableMapper& vm)
{
  bool negated = false;
  Node origin = n;
  if (n.getKind() == Kind::NOT)
  {
    Assert(n.getNumChildren() == 1)
        << "Expect negations to have a single child.";
    negated = true;
    n = *n.begin();
  }
  Assert((n.getKind() == Kind::EQUAL) || (n.getKind() == Kind::GT)
         || (n.getKind() == Kind::GEQ) || (n.getKind() == Kind::LT)
         || (n.getKind() == Kind::LEQ))
      << "Found a constraint with unsupported relation " << n.getKind();

  Assert(n.getNumChildren() == 2)
      << "Supported relations only have two children.";
  auto childit = n.begin();
  Integer ldenom;
  Polynomial left = as_poly_polynomial_impl(*childit++, ldenom, vm);
  Integer rdenom;
  Polynomial right = as_poly_polynomial_impl(*childit++, rdenom, vm);
  Assert(childit == n.end()) << "Screwed up iterator handling.";
  Assert(ldenom > Integer(0) && rdenom > Integer(0))
      << "Expected denominators to be always positive.";

  Integer g = gcd(ldenom, rdenom);
  Polynomial lhs = left * (rdenom/g) - right * (ldenom/g);
  SignCondition sc = normalize_kind(n.getKind(), negated, lhs);
  return {lhs, sc};
}

}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
