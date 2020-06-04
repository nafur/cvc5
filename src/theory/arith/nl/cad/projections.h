
#ifndef CVC4__THEORY__NLARITH__CAD_PROJECTIONS_H
#define CVC4__THEORY__NLARITH__CAD_PROJECTIONS_H

#include <algorithm>
#include <iostream>
#include <vector>

#include "../libpoly/polynomial.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace cad {

/** Sort and remove duplicates from the list of polynomials. */
void reduce_projection_polynomials(std::vector<libpoly::Polynomial>& polys);

/**
 * Adds a polynomial to the list of projection polynomials.
 * Before adding, it factorizes the polynomials and removed constant factors.
 */
void add_polynomial(std::vector<libpoly::Polynomial>& polys, const libpoly::Polynomial& poly);

void add_polynomials(std::vector<libpoly::Polynomial>& polys, const std::vector<libpoly::Polynomial>& p);

/**
 * Computes McCallum's projection operator.
 */
std::vector<libpoly::Polynomial> projection_mccallum(
    const std::vector<libpoly::Polynomial>& polys);

}  // namespace cad
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
