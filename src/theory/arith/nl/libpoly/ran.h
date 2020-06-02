
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__RAN_H
#define CVC4__THEORY__NLARITH__LIBPOLY__RAN_H

#include "assignment.h"
#include "integer.h"
#include "interval.h"
#include "polynomial.h"
#include "upolynomial.h"
#include "utils.h"
#include "value.h"

#include <poly/algebraic_number.h>

#include <iostream>

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

    /**
     * Implements a wrapper for lp_algebraic_number_t from libpoly.
     */
    class RAN {
        friend std::ostream& operator<<(std::ostream& os, const RAN& r);
        /** The actual algebraic number. */
        lp_algebraic_number_t mValue;
    public:
        /** Construct from a defining polynomial and an isolating interval. */
        RAN(UPolynomial&& poly, const Interval& i) {
            lp_algebraic_number_construct(&mValue, poly.release(), i.get());
        }
        /** Construct from a defining polynomial and an isolating interval. */
        RAN(const UPolynomial& poly, const Interval& i) {
            lp_algebraic_number_construct(&mValue, UPolynomial(poly).release(), i.get());
        }
        /** Construct from a lp_algebraic_number_t, copying its contents. */
        RAN(const lp_algebraic_number_t& ran) {
            lp_algebraic_number_construct_copy(&mValue, &ran);
        }
        /** Copy from the given RAN. */
        RAN(const RAN& ran) {
            lp_algebraic_number_construct_copy(&mValue, ran.get());
        }
        /** Move from the given RAN. */
        RAN(RAN&& ran) {
            lp_algebraic_number_construct_zero(&mValue);
            lp_algebraic_number_swap(&mValue, ran.get());
        }
        /** Custom destructor. */
        ~RAN() {
            lp_algebraic_number_destruct(&mValue);
        }
        /** Assign from the given RAN. */
        RAN& operator=(RAN r) {
            std::swap(mValue, r.mValue);
            return *this;
        }

        /** Implicitly convert to a Value. */
        operator Value() const {
            return Value(lp_value_new(lp_value_type_t::LP_VALUE_ALGEBRAIC, &mValue));
        }

        /** Get a non-const pointer to the internal lp_algebraic_number_t. Handle with care! */
        lp_algebraic_number_t* get() {
            return &mValue;
        }
        /** Get a const pointer to the internal lp_algebraic_number_t. */
        const lp_algebraic_number_t* get() const {
            return &mValue;
        }
    };
    /** Stream the given RAN to an output stream. */
    inline std::ostream& operator<<(std::ostream& os, const RAN& v) {
        return os << lp_algebraic_number_to_string(v.get());
    }

    /** Compare two RANs for equality. */
    inline bool operator==(const RAN& lhs, const RAN& rhs) {
        return lp_algebraic_number_cmp(lhs.get(), rhs.get()) == 0;
    }
    /** Compare a RAN and an Integer for equality. */
    inline bool operator==(const RAN& lhs, const Integer& rhs) {
        return lp_algebraic_number_cmp_integer(lhs.get(), rhs.get()) == 0;
    }
    /** Compare an Integer and a RAN for equality. */
    inline bool operator==(const Integer& lhs, const RAN& rhs) {
        return lp_algebraic_number_cmp_integer(rhs.get(), lhs.get()) == 0;
    }

    /** Isolate the real roots of a UPolynomial. */
    std::vector<RAN> isolate_real_roots(const UPolynomial& p);
    /** Isolate the real roots of a Polynomial with respect to an Assignment for all but the main variable. */
    std::vector<Value> isolate_real_roots(const Polynomial& p, const Assignment& a);

}
}
}
}
}

#endif
