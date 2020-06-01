
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__POLYNOMIAL_H
#define CVC4__THEORY__NLARITH__LIBPOLY__POLYNOMIAL_H

#include "utils.h"
#include "variable.h"

#include <poly/polynomial.h>

#include <iostream>

namespace CVC4 {
namespace theory {
namespace nlarith {
namespace libpoly {

    inline void polynomial_deleter(lp_polynomial_t* ptr) {
        lp_polynomial_delete(ptr);
    }

    class Polynomial {
        friend std::ostream& operator<<(std::ostream& os, const Polynomial& p);
    private:
        deleting_unique_ptr<lp_polynomial_t> mPoly;
    public:
        Polynomial(): mPoly(lp_polynomial_new(polynomial_ctx.get()), polynomial_deleter) {}
        Polynomial(lp_polynomial_t* poly): mPoly(poly, polynomial_deleter) {}
        Polynomial(int i, Variable v, unsigned n): mPoly(lp_polynomial_alloc(), polynomial_deleter) {
            lp_integer_t it;
            lp_integer_construct_from_int(lp_Z, &it, i);
            lp_polynomial_construct_simple(get(), polynomial_ctx.get(), &it, v.get(), n);
            lp_integer_destruct(&it);
        }
        Polynomial(Variable v): Polynomial(1, v, 1)  {}

        lp_polynomial_t* get() {
            return mPoly.get();
        }
        const lp_polynomial_t* get() const {
            return mPoly.get();
        }
    };

    // Standard operators

    inline std::ostream& operator<<(std::ostream& os, const Polynomial& p) {
        return os << lp_polynomial_to_string(p.get());
    }

    inline Polynomial operator+(const Polynomial& lhs, const Polynomial& rhs) {
        Polynomial res;
        lp_polynomial_add(res.get(), lhs.get(), rhs.get());
        return res;
    }
    inline Polynomial operator-(const Polynomial& lhs, const Polynomial& rhs) {
        Polynomial res;
        lp_polynomial_sub(res.get(), lhs.get(), rhs.get());
        return res;
    }
    inline Polynomial operator*(const Polynomial& lhs, const Polynomial& rhs) {
        Polynomial res;
        lp_polynomial_mul(res.get(), lhs.get(), rhs.get());
        return res;
    }

    inline Polynomial div(const Polynomial& lhs, const Polynomial& rhs) {
        Polynomial res;
        lp_polynomial_div(res.get(), lhs.get(), rhs.get());
        return res;
    }

    // Simple info operations

    inline std::size_t degree(const Polynomial& p) {
        return lp_polynomial_degree(p.get());
    }
    inline Polynomial leading_coefficient(const Polynomial& p) {
        Polynomial res;
        lp_polynomial_get_coefficient(res.get(), p.get(), degree(p));
        return res;
    }

    // Poly operations

    inline Polynomial derivative(const Polynomial& p) {
        Polynomial res;
        lp_polynomial_derivative(res.get(), p.get());
        return res;
    }

    inline Polynomial resultant(const Polynomial& p, const Polynomial& q) {
        Polynomial res;
        lp_polynomial_resultant(res.get(), p.get(), q.get());
        return res;
    }

    inline Polynomial discriminant(const Polynomial& p) {
        return div(resultant(p, derivative(p)), leading_coefficient(p));
    }


}
}
}
}

#endif