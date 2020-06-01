
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__INTERVAL_H
#define CVC4__THEORY__NLARITH__LIBPOLY__INTERVAL_H

#include "utils.h"

#include <poly/dyadic_interval.h>

#include <iostream>

namespace CVC4 {
namespace theory {
namespace nlarith {
namespace libpoly {

    class Interval {
        friend std::ostream& operator<<(std::ostream& os, const Interval& i);
    private:
        lp_dyadic_interval_t mInterval;
    public:
//        Polynomial(): mPoly(lp_polynomial_new(polynomial_ctx.get()), polynomial_deleter) {}
//        Polynomial(lp_polynomial_t* poly): mPoly(poly, polynomial_deleter) {}
//        Polynomial(int i, Variable v, unsigned n): mPoly(lp_polynomial_alloc(), polynomial_deleter) {
//            lp_integer_t it;
//            lp_integer_construct_from_int(lp_Z, &it, i);
//            lp_polynomial_construct_simple(get(), polynomial_ctx.get(), &it, v.get(), n);
//            lp_integer_destruct(&it);
//        }
        Interval(int a, int b) {
            lp_dyadic_interval_construct_from_int(&mInterval, a, 1, b, 1);
        }
        ~Interval() {
            lp_dyadic_interval_destruct(&mInterval);
        }

        lp_dyadic_interval_t& get() {
            return mInterval;
        }
        const lp_dyadic_interval_t& get() const {
            return mInterval;
        }
    };

    // Standard operators

    inline std::ostream& operator<<(std::ostream& os, const Interval& i) {
        os << (i.get().a_open ? "( " : "[ ");
        os << lp_dyadic_rational_to_string(&(i.get().a)) << " ; " << lp_dyadic_rational_to_string(&(i.get().b));
        os << (i.get().b_open ? " )" : " ]");
        return os;
    }

}
}
}
}

#endif