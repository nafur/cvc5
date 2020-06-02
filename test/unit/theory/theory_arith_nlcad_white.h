/*********************                                                        */
/*! \file theory_nl_white.h
 ** \verbatim
 ** Top contributors (to current version):
 **   Gereon Kremer
 ** This file is part of the CVC4 project.
 ** Copyright (c) 2009-2019 by the authors listed in the file AUTHORS
 ** in the top-level source directory) and their institutional affiliations.
 ** All rights reserved.  See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **
 ** \brief Unit tests for the nonlinear arithmetic
 **
 ** Unit tests for the nonlinear arithmetic.
 **/

#include <cxxtest/TestSuite.h>

#include <iostream>
#include <memory>
#include <vector>

#include "theory/arith/nl/libpoly/assignment.h"
#include "theory/arith/nl/libpoly/interval.h"
#include "theory/arith/nl/libpoly/polynomial.h"
#include "theory/arith/nl/libpoly/ran.h"
#include "theory/arith/nl/libpoly/upolynomial.h"
#include "theory/arith/nl/libpoly/value.h"
#include "theory/arith/nl/libpoly/variable.h"

#include "theory/arith/nl/cad_projections.h"

using namespace CVC4;
using namespace CVC4::theory::nl;

libpoly::UPolynomial get_upoly(std::initializer_list<int> init) {
    int coeffs[init.size()];
    std::size_t cur = 0;
    for (int c: init) coeffs[cur++] = c;
    return libpoly::UPolynomial(init.size()-1, coeffs);
}
libpoly::RAN get_ran(std::initializer_list<int> init, int lower, int upper) {
    return libpoly::RAN(get_upoly(init), libpoly::Interval(lower, upper));
}

class TheoryArithNLCADWhite : public CxxTest::TestSuite
{
 public:
  TheoryArithNLCADWhite() {}

  void setUp() override
  {
  }

  void tearDown() override {}

  void test_univariate_isolation()
  {
    libpoly::UPolynomial poly = get_upoly({-2, 2, 3, -3, -1, 1});
    auto roots = libpoly::isolate_real_roots(poly);

    TS_ASSERT(roots.size() == 4);
    TS_ASSERT(roots[0] == get_ran({-2, 0, 1}, -2, -1));
    TS_ASSERT(roots[1] == libpoly::Integer(-1))
    TS_ASSERT(roots[2] == libpoly::Integer(1));
    TS_ASSERT(roots[3] == get_ran({-2, 0, 1}, 1, 2));
  }

  void test_multivariate_isolation()
  {
    libpoly::Variable x("x");
    libpoly::Variable y("y");
    libpoly::Variable z("z");

    libpoly::Assignment a;
    a.set(x, get_ran({-2, 0, 1}, 1, 2));
    a.set(y, get_ran({-2, 0, 0, 0, 1}, 1, 2));

    libpoly::Polynomial poly = (y*y + x) - z;

    auto roots = libpoly::isolate_real_roots(poly, a);
    
    TS_ASSERT(roots.size() == 1);
    TS_ASSERT(roots[0] == get_ran({-8, 0, 1}, 2, 3));
  }

  void test_univariate_factorization()
  {
    libpoly::UPolynomial poly = get_upoly({-24, 44, -18, -1, 1, -3, 1});
    
    auto factors = square_free_factors(poly);
    std::cout << "Factors of " << poly << std::endl;
    for (const auto& f: factors) {
      std::cout << "---> " << f << std::endl;
    }
  }

  void test_projection()
  {
    // Gereon's thesis, Ex 5.1
    libpoly::Variable x("x");
    libpoly::Variable y("y");

    libpoly::Polynomial p = (y + 1)*(y + 1) - x*x*x + 3*x - 2;
    libpoly::Polynomial q = (x + 1)*y - 3;

    std::cout << "Executing McCallum" << std::endl;
    auto res = cad::projection_mccallum({p, q});
    for (const auto& r: res) {
      std::cout << "-> " << r << std::endl;
    }
  }

};
