
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__RAN_H
#define CVC4__THEORY__NLARITH__LIBPOLY__RAN_H

#include "assignment.h"
#include "interval.h"
#include "polynomial.h"
#include "upolynomial.h"
#include "utils.h"
#include "value.h"

#include <poly/algebraic_number.h>

#include <iostream>

namespace CVC4 {
namespace theory {
namespace nlarith {
namespace libpoly {

    class RAN {
        friend std::ostream& operator<<(std::ostream& os, const RAN& r);
        lp_algebraic_number_t mValue;
    public:
        RAN(UPolynomial&& poly, const Interval& i) {
            lp_algebraic_number_construct(&mValue, poly.release(), &i.get());
        }
        RAN(const UPolynomial& poly, const Interval& i) {
            lp_algebraic_number_construct(&mValue, UPolynomial(poly).release(), &i.get());
        }
        RAN(const lp_algebraic_number_t& ran) {
            lp_algebraic_number_construct_copy(&mValue, &ran);
        }
        RAN(const RAN& ran) {
            lp_algebraic_number_construct_copy(&mValue, &ran.get());
        }
        RAN(RAN&& ran) {
            lp_algebraic_number_construct_zero(&mValue);
            lp_algebraic_number_swap(&mValue, &ran.get());
        }
        ~RAN() {
            lp_algebraic_number_destruct(&mValue);
        }

        operator Value() const {
            return Value(lp_value_new(lp_value_type_t::LP_VALUE_ALGEBRAIC, &mValue));
        }

        lp_algebraic_number_t& get() {
            return mValue;
        }
        const lp_algebraic_number_t& get() const {
            return mValue;
        }
    };
    inline std::ostream& operator<<(std::ostream& os, const RAN& v) {
        return os << lp_algebraic_number_to_string(&(v.get()));
    }

    inline bool operator==(const RAN& lhs, const RAN& rhs) {
        return lp_algebraic_number_cmp(&lhs.get(), &rhs.get()) == 0;
    }
    inline bool operator==(const RAN& lhs, const lp_integer_t& rhs) {
        return lp_algebraic_number_cmp_integer(&lhs.get(), &rhs) == 0;
    }
    inline bool operator==(const lp_integer_t& lhs, const RAN& rhs) {
        return lp_algebraic_number_cmp_integer(&rhs.get(), &lhs) == 0;
    }

    std::vector<RAN> isolate_real_roots(const UPolynomial& p);
    std::vector<Value> isolate_real_roots(const Polynomial& p, const Assignment& a);

}
}
}
}

#endif