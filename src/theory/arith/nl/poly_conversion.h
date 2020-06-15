
#ifndef CVC4__THEORY__ARITH__NL__POLY_CONVERSION_H
#define CVC4__THEORY__ARITH__NL__POLY_CONVERSION_H

#include <poly/polyxx.h>

#include <iostream>

#include "expr/node.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {

/** Bijective mapping between CVC4 variables and poly variables.
 */
struct VariableMapper
{
  /** A mapping from CVC4 variables to poly variables.
   */
  std::map<CVC4::Node, poly::Variable> mVarCVCpoly;
  /** A mapping from poly variables to CVC4 variables. */
  std::map<poly::Variable, CVC4::Node> mVarpolyCVC;

  /** Retrieves the according poly variable. */
  poly::Variable operator()(const CVC4::Node& n);
  /** Retrieves the according CVC4 variable. */
  CVC4::Node operator()(const poly::Variable& n);
};

/** Convert poly univariate polynomial to a CVC4::Node. */
CVC4::Node as_cvc_polynomial(const poly::UPolynomial& p, const CVC4::Node& var);

/** Constructs a polynomial from the given node.
 *
 * While a Node may contain rationals, a Polynomial does not.
 * We therefore also store the denominator of the returned polynomial and
 * use it to construct the integer polynomial recursively.
 * Once the polynomial has been fully constructed, we can ignore the
 * denominator (except for its sign, which is always positive, though).
 */
poly::Polynomial as_poly_polynomial(const CVC4::Node& n, VariableMapper& vm);

/** Constructs a constraints (a polynomial and a sign condition) from the given
 * node.
 */
std::pair<poly::Polynomial, poly::SignCondition> as_poly_constraint(
    Node n, VariableMapper& vm);

}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
