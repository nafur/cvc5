#include "poly_conversion.h"

#include "expr/node.h"
#include "expr/node_manager_attributes.h"
#include "util/integer.h"
#include "util/poly_util.h"
#include "util/rational.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {

poly::Variable VariableMapper::operator()(const CVC4::Node& n)
{
  Assert(n.isVar()) << "Expect node to be a variable.";
  auto it = mVarCVCpoly.find(n);
  if (it == mVarCVCpoly.end())
  {
    std::string name;
    if (!n.getAttribute(expr::VarNameAttr(), name))
    {
      Trace("poly::conversion")
          << "Variable " << n << " has no name, using ID instead." << std::endl;
      name = "v_" + std::to_string(n.getId());
    }
    it = mVarCVCpoly.emplace(n, poly::Variable(name.c_str())).first;
    mVarpolyCVC.emplace(it->second, n);
  }
  return it->second;
}

CVC4::Node VariableMapper::operator()(const poly::Variable& n)
{
  auto it = mVarpolyCVC.find(n);
  Assert(it != mVarpolyCVC.end()) << "Expect variable to be added already.";
  return it->second;
}

CVC4::Node as_cvc_polynomial(const poly::UPolynomial& p, const CVC4::Node& var)
{
  Trace("poly::conversion")
      << "Converting " << p << " over " << var << std::endl;

  std::size_t size = degree(p) + 1;
  poly::Integer coeffs[size];

  lp_upolynomial_unpack(p.get_internal(), poly::detail::cast_to(coeffs));
  auto* nm = NodeManager::currentNM();

  Node res = nm->mkConst(Rational(0));
  Node monomial = nm->mkConst(Rational(1));
  for (std::size_t i = 0; i < size; ++i)
  {
    if (is_zero(coeffs[i]))
    {
      Node coeff = nm->mkConst(Rational(poly_utils::to_integer(coeffs[i])));
      Node term = nm->mkNode(Kind::MULT, coeff, monomial);
      res = nm->mkNode(Kind::PLUS, res, term);
    }
    monomial = nm->mkNode(Kind::NONLINEAR_MULT, monomial, var);
  }
  Trace("poly::conversion") << "-> " << res << std::endl;
  return res;
}


poly::Polynomial as_poly_polynomial_impl(const CVC4::Node& n,
                                   poly::Integer& denominator,
                                   VariableMapper& vm)
{
  denominator = poly::Integer(1);
  if (n.isVar()) {
    return poly::Polynomial(vm(n));
  }
  switch (n.getKind())
  {
    case Kind::CONST_RATIONAL:
    {
      Rational r = n.getConst<Rational>();
      denominator = poly_utils::to_integer(r.getDenominator());
      return poly::Polynomial(poly_utils::to_integer(r.getNumerator()));
    }
    case Kind::PLUS:
    {
      poly::Polynomial res;
      poly::Integer denom;
      for (const auto& child : n)
      {
        poly::Polynomial tmp = as_poly_polynomial_impl(child, denom, vm);
        /** Normalize denominators
         */
        poly::Integer g = gcd(denom, denominator);
        res = res * (denom/g) + tmp * (denominator/g);
        denominator *= (denom/g);
      }
      return res;
    }
    case Kind::MULT:
    case Kind::NONLINEAR_MULT:
    {
      poly::Polynomial res(denominator);
      poly::Integer denom;
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
  return poly::Polynomial();
}
poly::Polynomial as_poly_polynomial(const CVC4::Node& n, VariableMapper& vm)
{
  poly::Integer denom;
  return as_poly_polynomial_impl(n, denom, vm);
}

poly::SignCondition normalize_kind(CVC4::Kind kind,
                                      bool negated,
                                      poly::Polynomial& lhs)
{
  switch (kind)
  {
    case Kind::EQUAL:
    {
      return negated ? poly::SignCondition::NE : poly::SignCondition::EQ;
    }
    case Kind::LT:
    {
      if (negated)
      {
        lhs = -lhs;
        return poly::SignCondition::LE;
      }
      return poly::SignCondition::LT;
    }
    case Kind::LEQ:
    {
      if (negated)
      {
        lhs = -lhs;
        return poly::SignCondition::LT;
      }
      return poly::SignCondition::LE;
    }
    case Kind::GT:
    {
      if (negated)
      {
        return poly::SignCondition::LE;
      }
      lhs = -lhs;
      return poly::SignCondition::LT;
    }
    case Kind::GEQ:
    {
      if (negated)
      {
        return poly::SignCondition::LT;
      }
      lhs = -lhs;
      return poly::SignCondition::LE;
    }
    default:
      Assert(false) << "This function only deals with arithmetic relations.";
      return poly::SignCondition::EQ;
  }
}

std::pair<poly::Polynomial, poly::SignCondition> as_poly_constraint(
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
  poly::Integer ldenom;
  poly::Polynomial left = as_poly_polynomial_impl(*childit++, ldenom, vm);
  poly::Integer rdenom;
  poly::Polynomial right = as_poly_polynomial_impl(*childit++, rdenom, vm);
  Assert(childit == n.end()) << "Screwed up iterator handling.";
  Assert(ldenom > poly::Integer(0) && rdenom > poly::Integer(0))
      << "Expected denominators to be always positive.";

  poly::Integer g = gcd(ldenom, rdenom);
  poly::Polynomial lhs = left * (rdenom/g) - right * (ldenom/g);
  poly::SignCondition sc = normalize_kind(n.getKind(), negated, lhs);
  return {lhs, sc};
}

}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
