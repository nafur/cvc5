#include "cvc4autoconfig.h"
#include "util/real_algebraic_number.h"

#ifndef CVC4_POLY_IMP  // Make sure this comes after cvc4autoconfig.h
#error "This source should only ever be built if CVC4_POLY_IMP is on !"
#endif /* CVC4_POLY_IMP */

#include "base/check.h"
#include "poly_util.h"

#include "poly/polyxx.h"

namespace CVC4 {

RealAlgebraicNumber::RealAlgebraicNumber() {}

RealAlgebraicNumber::RealAlgebraicNumber(poly::AlgebraicNumber&& an): d_value(std::move(an)) {}

RealAlgebraicNumber::RealAlgebraicNumber(const Integer& i)
    : d_value(poly::DyadicRational(poly::Integer(i.getValue())))
{
}

RealAlgebraicNumber::RealAlgebraicNumber(const Rational& r)
{
  poly::Rational pr(r.getValue());
  auto dr = poly_utils::to_dyadic_rational(r);
  if (dr) {
    d_value = poly::AlgebraicNumber(
        poly::UPolynomial({numerator(pr), -denominator(pr)}),
        poly::DyadicInterval(dr.value()));
  } else {
    d_value = poly::AlgebraicNumber(
        poly::UPolynomial({numerator(pr), -denominator(pr)}),
        poly::DyadicInterval(floor(pr), ceil(pr)));
  }
}

RealAlgebraicNumber::RealAlgebraicNumber(
    const std::vector<Integer>& coefficients,
    const Rational& lower,
    const Rational& upper)
{
    *this = poly_utils::from_rationals_with_refinement(poly::UPolynomial(poly_utils::to_integer(coefficients)), lower, upper);
}
RealAlgebraicNumber::RealAlgebraicNumber(
    const std::vector<Rational>& coefficients,
    const Rational& lower,
    const Rational& upper)
{
    Integer factor = Integer(1);
    for (const auto& c: coefficients) {
        factor = factor.lcm(c.getDenominator());
    }
    std::vector<poly::Integer> coeffs;
    for (const auto& c: coefficients) {
        Assert((c * factor).getDenominator() == Integer(1));
        coeffs.emplace_back((c * factor).getNumerator().getValue());
    }
    *this = poly_utils::from_rationals_with_refinement(poly::UPolynomial(std::move(coeffs)), lower, upper);
}

RealAlgebraicNumber& RealAlgebraicNumber::operator=(const RealAlgebraicNumber& ran) {
    d_value = ran.getValue();
    return *this;
}
RealAlgebraicNumber& RealAlgebraicNumber::operator=(RealAlgebraicNumber&& ran) {
    d_value = std::move(ran.getValue());
    return *this;
}

std::ostream& operator<<(std::ostream& os, const RealAlgebraicNumber& ran) {
    return os << ran.getValue();
}

bool operator==(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs) {
    return lhs.getValue() == rhs.getValue();
}
bool operator!=(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs) {
    return lhs.getValue() != rhs.getValue();
}
bool operator<(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs) {
    return lhs.getValue() < rhs.getValue();
}
bool operator<=(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs) {
    return lhs.getValue() <= rhs.getValue();
}
bool operator>(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs) {
    return lhs.getValue() > rhs.getValue();
}
bool operator>=(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs) {
    return lhs.getValue() >= rhs.getValue();
}

RealAlgebraicNumber operator+(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs) {
    return lhs.getValue() + rhs.getValue();
}
RealAlgebraicNumber operator-(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs) {
    return lhs.getValue() - rhs.getValue();
}
RealAlgebraicNumber operator-(const RealAlgebraicNumber& ran) {
    return -ran.getValue();
}
RealAlgebraicNumber operator*(const RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs) {
    return lhs.getValue() * rhs.getValue();
}

RealAlgebraicNumber& operator+=(RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs) {
    lhs.getValue() = lhs.getValue() + rhs.getValue();
    return lhs;
}
RealAlgebraicNumber& operator-=(RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs) {
    lhs.getValue() = lhs.getValue() - rhs.getValue();
    return lhs;
}
RealAlgebraicNumber& operator*=(RealAlgebraicNumber& lhs, const RealAlgebraicNumber& rhs) {
    lhs.getValue() = lhs.getValue() * rhs.getValue();
    return lhs;
}

int sgn(const RealAlgebraicNumber& ran) {
    return sgn(ran.getValue());
}
bool is_zero(const RealAlgebraicNumber& ran) {
    return is_zero(ran.getValue());
}
bool is_one(const RealAlgebraicNumber& ran) {
    return is_one(ran.getValue());
}


}  // namespace CVC4
