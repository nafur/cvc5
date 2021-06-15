/******************************************************************************
 * Top contributors (to current version):
 *   Aina Niemetz, Tim King
 *
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2021 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * White box testing of cvc5::Integer.
 */

#include <sstream>

#include "test.h"
#include "util/interval.h"

namespace cvc5 {
namespace test {

class TestUtilWhiteInterval : public TestInternal
{
};

#define EXPECT_SUBSET(a, b) \
  EXPECT_PRED2(isSubset, a, Interval(b - 0.01, b + 0.01))
#define EXPECT_EMPTY(a) \
  EXPECT_PRED1(isEmpty, a)
#define EXPECT_WHOLE(a) \
  EXPECT_PRED1(isWhole, a)

TEST_F(TestUtilWhiteInterval, trig_functions)
{
  EXPECT_SUBSET(abs(Interval(0)), 0);
  EXPECT_SUBSET(abs(Interval(1)), 1);
  EXPECT_SUBSET(abs(Interval(2)), 2);

  EXPECT_SUBSET(exp(Interval(0)), 1);
  EXPECT_SUBSET(exp(Interval(1)), 2.7183);
  EXPECT_SUBSET(exp(Interval(2)), 7.389);

  EXPECT_SUBSET(sqrt(Interval(0)), 0);
  EXPECT_SUBSET(sqrt(Interval(1)), 1);
  EXPECT_SUBSET(sqrt(Interval(2)), 1.414);

  std::cout << "Trig(0)" << std::endl;
  EXPECT_SUBSET(sin(Interval(0)), 0);
  EXPECT_SUBSET(cos(Interval(0)), 1);
  EXPECT_SUBSET(tan(Interval(0)), 0);
  EXPECT_WHOLE(cosec(Interval(0)));
  EXPECT_SUBSET(sec(Interval(0)), 1);
  EXPECT_WHOLE(cotan(Interval(0)));
  EXPECT_SUBSET(asin(Interval(0)), 0);
  EXPECT_SUBSET(acos(Interval(0)), 1.5708);
  EXPECT_SUBSET(atan(Interval(0)), 0);
  EXPECT_WHOLE(acosec(Interval(0)));
  EXPECT_WHOLE(asec(Interval(0)));
  EXPECT_SUBSET(acotan(Interval(0)), 1.5708);

  std::cout << "Trig(1)" << std::endl;
  EXPECT_SUBSET(sin(Interval(1)), 0.841);
  EXPECT_SUBSET(cos(Interval(1)), 0.540);
  EXPECT_SUBSET(tan(Interval(1)), 1.557);
  EXPECT_SUBSET(cosec(Interval(1)), 1.188);
  EXPECT_SUBSET(sec(Interval(1)), 1.850);
  EXPECT_SUBSET(cotan(Interval(1)), 0.642);
  EXPECT_SUBSET(asin(Interval(1)), 1.570);
  EXPECT_SUBSET(acos(Interval(1)), 0);
  EXPECT_SUBSET(atan(Interval(1)), 0.785);
  EXPECT_SUBSET(acosec(Interval(1)), 1.571);
  EXPECT_SUBSET(asec(Interval(1)), 0);
  EXPECT_SUBSET(acotan(Interval(1)), 0.785);

  std::cout << "Trig(2)" << std::endl;
  EXPECT_SUBSET(sin(Interval(2)), 0.909);
  EXPECT_SUBSET(cos(Interval(2)), -0.416);
  EXPECT_SUBSET(tan(Interval(2)), -2.185);
  EXPECT_SUBSET(cosec(Interval(2)), 1.099);
  EXPECT_SUBSET(sec(Interval(2)), -2.403);
  EXPECT_SUBSET(cotan(Interval(2)), -0.457);
  EXPECT_EMPTY(asin(Interval(2)));
  EXPECT_EMPTY(acos(Interval(2)));
  EXPECT_SUBSET(atan(Interval(2)), 1.107);
  EXPECT_SUBSET(acosec(Interval(2)), 0.523);
  EXPECT_SUBSET(asec(Interval(2)), 1.047);
  EXPECT_SUBSET(acotan(Interval(2)), 0.463);
}

}  // namespace test
}  // namespace cvc5
