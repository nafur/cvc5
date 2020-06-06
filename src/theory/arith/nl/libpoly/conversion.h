
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__CONVERSION_H
#define CVC4__THEORY__NLARITH__LIBPOLY__CONVERSION_H

#include <poly/dyadic_rational.h>
#include <poly/poly.h>

#include <iostream>

#include "expr/node.h"
#include "integer.h"
#include "polynomial.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/** Bijective mapping between CVC4 variables and libpoly variables.
 */
struct VariableMapper
{
  /** A mapping from CVC4 variables to libpoly variables.
   */
  std::map<CVC4::Node, libpoly::Variable> mVarCVCpoly;
  /** A mapping from libpoly variables to CVC4 variables. */
  std::map<libpoly::Variable, CVC4::Node> mVarpolyCVC;

  /** Retrieves the according libpoly variable. */
  libpoly::Variable operator()(const CVC4::Node& n);
  /** Retrieves the according CVC4 variable. */
  CVC4::Node operator()(const libpoly::Variable& n);
};

/** Convert libpoly integer to CVC4::Integer. */
CVC4::Integer as_cvc_integer(const lp_integer_t* v);
/** Convert libpoly rational to CVC4::Rational. */
CVC4::Rational as_cvc_rational(const lp_rational_t* v);
/** Convert libpoly dyadic rational to CVC4::Rational. */
CVC4::Rational as_cvc_rational(const lp_dyadic_rational_t* v);

/** Convert libpoly univariate polynomial to a CVC4::Node. */
CVC4::Node as_cvc_polynomial(const UPolynomial& p, const CVC4::Node& var);

/** Constructs a polynomial from the given node.
 *
 * While a Node may contain rationals, a Polynomial does not.
 * We therefore also store the denominator of the returned polynomial and
 * use it to construct the integer polynomial recursively.
 * Once the polynomial has been fully constructed, we can ignore the
 * denominator (except for its sign, which is always positive, though).
 */
Polynomial as_poly_polynomial(const CVC4::Node& n, VariableMapper& vm);

/** Constructs a constraints (a polynomial and a sign condition) from the given
 * node.
 */
std::pair<libpoly::Polynomial, libpoly::SignCondition> as_poly_constraint(
    Node n, VariableMapper& vm);

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
