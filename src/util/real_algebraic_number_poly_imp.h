/*********************                                                        */
/*! \file rational_gmp_imp.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Tim King, Morgan Deters, Christopher L. Conway
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Algebraic number constants; wraps a libpoly algebraic number.
 **
 ** Algebraic number constants; wraps a libpoly algebraic number.
 **/

#include "cvc4_public.h"

#ifndef CVC4__REAL_ALGEBRAIC_NUMBER_H
#define CVC4__REAL_ALGEBRAIC_NUMBER_H

#include "poly/polyxx.h"

#include "util/integer.h"
#include "util/rational.h"

#include <vector>

namespace CVC4 {

class CVC4_PUBLIC RealAlgebraicNumber {
private:
  poly::AlgebraicNumber d_value;

public:

  RealAlgebraicNumber();
  RealAlgebraicNumber(poly::AlgebraicNumber&& an);
  RealAlgebraicNumber(const Integer& i);
  RealAlgebraicNumber(const Rational& r);
  RealAlgebraicNumber(const std::vector<Integer>& coefficients, const Rational& lower, const Rational& upper);
  RealAlgebraicNumber(const std::vector<Rational>& coefficients, const Rational& lower, const Rational& upper);

  RealAlgebraicNumber(const RealAlgebraicNumber& ran);
  RealAlgebraicNumber(RealAlgebraicNumber&& ran);

  ~RealAlgebraicNumber() {}

  RealAlgebraicNumber& operator=(const RealAlgebraicNumber& ran);
  RealAlgebraicNumber& operator=(RealAlgebraicNumber&& ran);

  const poly::AlgebraicNumber& getValue() const
  {
    return d_value;
  }
  poly::AlgebraicNumber& getValue()
  {
    return d_value;
  }

};/* class RealAlgebraicNumber */

CVC4_PUBLIC std::ostream& operator<<(std::ostream& os, const RealAlgebraicNumber& ran);

CVC4_PUBLIC bool operator==(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs);
CVC4_PUBLIC bool operator!=(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs);
CVC4_PUBLIC bool operator<(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs);
CVC4_PUBLIC bool operator<=(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs);
CVC4_PUBLIC bool operator>(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs);
CVC4_PUBLIC bool operator>=(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs);

CVC4_PUBLIC RealAlgebraicNumber operator+(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs);
CVC4_PUBLIC RealAlgebraicNumber operator-(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs);
CVC4_PUBLIC RealAlgebraicNumber operator-(const RealAlgebraicNumber& ran);
CVC4_PUBLIC RealAlgebraicNumber operator*(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs);

CVC4_PUBLIC RealAlgebraicNumber& operator+=(RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs);
CVC4_PUBLIC RealAlgebraicNumber& operator-=(RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs);
CVC4_PUBLIC RealAlgebraicNumber& operator*=(RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs);

CVC4_PUBLIC int sgn(const RealAlgebraicNumber& ran);
CVC4_PUBLIC bool is_zero(const RealAlgebraicNumber& ran);
CVC4_PUBLIC bool is_one(const RealAlgebraicNumber& ran);

}/* CVC4 namespace */

#endif /* CVC4__REAL_ALGEBRAIC_NUMBER_H */
