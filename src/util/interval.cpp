/******************************************************************************
 * Top contributors (to current version):
 *   Gereon Kremer
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

#include "util/interval.h"

#include <iostream>
#include <boost/numeric/interval.hpp>
#include <boost/numeric/interval/io.hpp>

#include "base/check.h"

namespace cvc5 {

namespace bn = boost::numeric;

struct IntervalState {
    using interval = bn::interval<double,
        bn::interval_lib::policies<
            bn::interval_lib::save_state<
                bn::interval_lib::rounded_transc_std<double>
            >,
            bn::interval_lib::checking_base<double>
        >
    >;

    IntervalState(double low, double high): d_interval(low, high) {}
    IntervalState(const interval& i): d_interval(i) {}
    IntervalState(interval&& i): d_interval(std::move(i)) {}

    interval d_interval;
};

Interval toInterval(IntervalState::interval&& i) {
    return Interval(std::make_unique<IntervalState>(std::move(i)));
}

Interval::Interval(double low, double high):
    d_state(std::make_unique<IntervalState>(low, high)) {}
Interval::Interval(double point): Interval(point, point) {}
Interval::Interval(const Interval& i):
    d_state(std::make_unique<IntervalState>(i.d_state->d_interval)) {}
Interval::Interval(std::unique_ptr<IntervalState>&& i):
    d_state(std::move(i)) {}

Interval::~Interval() {}

Interval& Interval::operator=(const Interval& i) {
    d_state->d_interval = i.d_state->d_interval;
    return *this;
}
Interval& Interval::operator=(Interval&& i) {
    std::swap(d_state, i.d_state);
    return *this;
}

Interval Interval::pi()
{
    return toInterval(bn::interval_lib::pi<IntervalState::interval>());
}

std::ostream& operator<<(std::ostream& os, const Interval& i) {
    return os << i.state().d_interval;
}

Interval operator-(const Interval& rhs) {
    return toInterval(-rhs.state().d_interval);
}

Interval operator+(const Interval& lhs, const Interval& rhs) {
    return toInterval(lhs.state().d_interval + rhs.state().d_interval);
}
Interval operator-(const Interval& lhs, const Interval& rhs) {
    return toInterval(lhs.state().d_interval - rhs.state().d_interval);
}
Interval operator*(const Interval& lhs, const Interval& rhs) {
    return toInterval(lhs.state().d_interval * rhs.state().d_interval);
}
Interval operator/(const Interval& lhs, const Interval& rhs) {
    return toInterval(lhs.state().d_interval / rhs.state().d_interval);
}

Interval abs(const Interval& rhs) {
    return toInterval(bn::abs(rhs.state().d_interval));
}
Interval exp(const Interval& rhs) {
    return toInterval(bn::exp(rhs.state().d_interval));
}
Interval sin(const Interval& rhs) {
    return toInterval(bn::sin(rhs.state().d_interval));
}
Interval cos(const Interval& rhs) {
    return toInterval(bn::cos(rhs.state().d_interval));
}
Interval tan(const Interval& rhs) {
    return toInterval(bn::tan(rhs.state().d_interval));
}
Interval cosec(const Interval& rhs) {
    auto tmp = bn::sin(rhs.state().d_interval);
    if (bn::zero_in(tmp)) {
        return toInterval(IntervalState::interval::whole());
    }
    return toInterval(bn::interval_lib::multiplicative_inverse(tmp));
}
Interval sec(const Interval& rhs) {
    auto tmp = bn::cos(rhs.state().d_interval);
    if (bn::zero_in(tmp)) {
        return toInterval(IntervalState::interval::whole());
    }
    return toInterval(bn::interval_lib::multiplicative_inverse(tmp));
}
Interval cotan(const Interval& rhs) {
    auto tmp = bn::tan(rhs.state().d_interval);
    if (bn::zero_in(tmp)) {
        return toInterval(IntervalState::interval::whole());
    }
    return toInterval(bn::interval_lib::multiplicative_inverse(tmp));
}
Interval asin(const Interval& rhs) {
    return toInterval(bn::asin(rhs.state().d_interval));
}
Interval acos(const Interval& rhs) {
    return toInterval(bn::acos(rhs.state().d_interval));
}
Interval atan(const Interval& rhs) {
    return toInterval(bn::atan(rhs.state().d_interval));
}
Interval acosec(const Interval& rhs) {
    // arcsin(1/x) = arccosec(x)
    if (bn::zero_in(rhs.state().d_interval)) {
        return toInterval(IntervalState::interval::whole());
    }
    return toInterval(
        bn::asin(bn::interval_lib::multiplicative_inverse(rhs.state().d_interval)));
}
Interval asec(const Interval& rhs) {
    // arccos(1/x) = arcsec(x)
    if (bn::zero_in(rhs.state().d_interval)) {
        return toInterval(IntervalState::interval::whole());
    }
    return toInterval(
        bn::acos(bn::interval_lib::multiplicative_inverse(rhs.state().d_interval)));
}
Interval acotan(const Interval& rhs) {
    // arccot(x) ) pi/2 - arctan(x)
    return toInterval(
        bn::interval_lib::pi_half<IntervalState::interval>() - bn::atan(rhs.state().d_interval));
}
Interval sqrt(const Interval& rhs) {
    return toInterval(bn::sqrt(rhs.state().d_interval));
}

bool isEmpty(const Interval& rhs) {
    return bn::empty(rhs.state().d_interval);
}
bool isWhole(const Interval& rhs) {
    const auto& i = rhs.state().d_interval;
    return std::isinf(i.lower()) && std::isinf(i.upper());
}
bool isSubset(const Interval& lhs, const Interval& rhs) {
    Assert(!bn::empty(lhs.state().d_interval));
    return bn::subset(lhs.state().d_interval, rhs.state().d_interval);
}

}  // namespace cvc5
