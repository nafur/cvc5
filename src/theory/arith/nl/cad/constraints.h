#ifndef CVC4__THEORY__NLARITH__CAD__CONSTRAINTS_H
#define CVC4__THEORY__NLARITH__CAD__CONSTRAINTS_H

#include <map>
#include <tuple>
#include <vector>

#include "../libpoly/polynomial.h"
#include "../libpoly/sign_condition.h"
#include "expr/kind.h"
#include "expr/node_manager_attributes.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

class Constraints
{
  /** Type alias for a list of constraints. */
  using ConstraintVector = std::vector<
      std::tuple<libpoly::Polynomial, libpoly::SignCondition, Node>>;
  /** A list of constraints, each comprised of a polynomial and a sign
   * condition.
   */
  ConstraintVector mConstraints;

  /** A mapping from CVC4 variables to libpoly variables.
   */
  std::map<Node, libpoly::Variable> mVarCVCpoly;
  /** A mapping from libpoly variables to CVC4 variables. */
  std::map<libpoly::Variable, Node> mVarpolyCVC;

  /** Get the corresponding libpoly variable or create a new one.
   * Expects the given node to be a Kind::VARIABLE.
   */
  libpoly::Variable var_cvc_to_poly(const Node& n);

  /** Checks whether the given relation can be handled by CAD.
   * Returns true if the given Kind is one of EQUAL, GT, GEQ, LT or LEQ.
   */
  bool is_suitable_relation(Kind kind) const;

  /** Normalizes two denominators.
   * Divides both by their gcd.
   */
  void normalize_denominators(libpoly::Integer& d1, libpoly::Integer& d2) const;

  /** Normalize the given kind to a SignCondition, taking negation into account.
   * Always normalizes to EQ, NE, LT or LE. Negates the polynomial is necessary.
   */
  libpoly::SignCondition normalize_kind(Kind kind,
                                        bool negated,
                                        libpoly::Polynomial& lhs) const;

  /** Constructs a polynomial from the given node.
   *
   * While a Node may contain rationals, a Polynomial does not.
   * We therefore also store the denominator of the returned polynomial and
   * use it to construct the integer polynomial recursively.
   * Once the polynomial has been fully constructed, we can ignore the
   * denominator (except for its sign, which is always positive, though).
   */
  libpoly::Polynomial construct_polynomial(const Node& n,
                                           libpoly::Integer& denominator);

  /** Constructs the lhs polynomial for a node representing a constraints.
   * Assumes the node to have exactly two children.
   */
  libpoly::Polynomial construct_constraint_polynomial(const Node& n);

 public:

  /** Get the corresponding CVC4 variable.
   * Expects that it was added to the internal mapping via var_cvc_to_poly beforehand.
   */
  Node var_poly_to_cvc(const libpoly::Variable& n) const;
  
  /** Add a constraing (represented by a polynomial and a sign condition) to the
   * list of constraints.
   */
  void add_constraint(const libpoly::Polynomial& lhs,
                      libpoly::SignCondition sc,
                      Node n);

  /** Add a constraints (represented by a node) to the list of constraints.
   * The given node can either be a negation (NOT) or a suitable relation symbol
   * as checked by is_suitable_relation().
   */
  void add_constraint(Node n);

  /** Gives the list of added constraints.
   */
  const ConstraintVector& get_constraints() const;

  /** Remove all constraints. */
  void reset();
};

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif