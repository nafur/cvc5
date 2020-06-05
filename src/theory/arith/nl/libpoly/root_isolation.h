
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__ROOT_ISOLATION_H
#define CVC4__THEORY__NLARITH__LIBPOLY__ROOT_ISOLATION_H

#include <poly/algebraic_number.h>

#include "assignment.h"
#include "polynomial.h"
#include "upolynomial.h"
#include "ran.h"
#include "value.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/** Isolate the real roots of a UPolynomial. */
std::vector<RAN> isolate_real_roots(const UPolynomial& p);
/** Isolate the real roots of a Polynomial with respect to an Assignment for all
 * but the main variable. */
std::vector<Value> isolate_real_roots(const Polynomial& p, const Assignment& a);

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
