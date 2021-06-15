/******************************************************************************
 * Top contributors (to current version):
 *   Gereon Kremer, Aina Niemetz
 *
 * This file is part of the cvc5 project.
 *
 * Copyright (c) 2009-2021 by the authors listed in the file AUTHORS
 * in the top-level source directory and their institutional affiliations.
 * All rights reserved.  See the file COPYING in the top-level source
 * directory for licensing information.
 * ****************************************************************************
 *
 * The Interval class.
 */

#include "cvc5_private.h"

#ifndef CVC5__INTERVAL_H
#define CVC5__INTERVAL_H

#include <iosfwd>
#include <memory>

namespace cvc5 {

struct IntervalState;
class Interval {
public:
    Interval(double low, double high);
    Interval(double point);
    Interval(const Interval& i);
    Interval(std::unique_ptr<IntervalState>&& i);
    ~Interval();
    
    Interval& operator=(const Interval& i);
    Interval& operator=(Interval&& i);

    const IntervalState& state() const {
        return *d_state;
    }

    static Interval pi();
private:
    std::unique_ptr<IntervalState> d_state;
};

std::ostream& operator<<(std::ostream& os, const Interval& i);

Interval operator-(const Interval& rhs);

Interval operator+(const Interval& lhs, const Interval& rhs);
Interval operator-(const Interval& lhs, const Interval& rhs);
Interval operator*(const Interval& lhs, const Interval& rhs);
Interval operator/(const Interval& lhs, const Interval& rhs);

Interval abs(const Interval& rhs);
Interval exp(const Interval& rhs);
Interval sin(const Interval& rhs);
Interval cos(const Interval& rhs);
Interval tan(const Interval& rhs);
Interval cosec(const Interval& rhs);
Interval sec(const Interval& rhs);
Interval cotan(const Interval& rhs);
Interval asin(const Interval& rhs);
Interval acos(const Interval& rhs);
Interval atan(const Interval& rhs);
Interval acosec(const Interval& rhs);
Interval asec(const Interval& rhs);
Interval acotan(const Interval& rhs);
Interval sqrt(const Interval& rhs);

bool isWhole(const Interval& rhs);
bool isEmpty(const Interval& rhs);
bool isSubset(const Interval& lhs, const Interval& rhs);

}  // namespace cvc5

#endif /* CVC5__INTERVAL_H */
