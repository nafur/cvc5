
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__UPOLYNOMIAL_H
#define CVC4__THEORY__NLARITH__LIBPOLY__UPOLYNOMIAL_H

#include "utils.h"
#include "variable.h"

#include <poly/integer.h>
#include <poly/upolynomial.h>

#include <iostream>
#include <vector>

namespace CVC4 {
namespace theory {
namespace nlarith {
namespace libpoly {

    inline void upolynomial_deleter(lp_upolynomial_t* ptr) {
        lp_upolynomial_delete(ptr);
    }

    class UPolynomial {
        friend std::ostream& operator<<(std::ostream& os, const UPolynomial& p);
    private:
        deleting_unique_ptr<lp_upolynomial_t> mPoly;
    public:
        UPolynomial(const UPolynomial& poly): mPoly(lp_upolynomial_construct_copy(poly.get()), upolynomial_deleter) {}
        UPolynomial(lp_upolynomial_t* poly): mPoly(poly, upolynomial_deleter) {}
        UPolynomial(std::size_t degree, const int* coeffs): mPoly(lp_upolynomial_construct_from_int(lp_Z, degree, coeffs), upolynomial_deleter) {}
        UPolynomial(std::size_t degree, const lp_integer_t* coeffs): mPoly(lp_upolynomial_construct(lp_Z, degree, coeffs), upolynomial_deleter) {}

        lp_upolynomial_t* get() {
            return mPoly.get();
        }
        const lp_upolynomial_t* get() const {
            return mPoly.get();
        }
        lp_upolynomial_t* release() {
            return mPoly.release();
        }
    };

    // Standard operators

    inline std::ostream& operator<<(std::ostream& os, const UPolynomial& p) {
        return os << lp_upolynomial_to_string(p.get());
    }

    inline std::size_t degree(const UPolynomial& p) {
        return lp_upolynomial_degree(p.get());
    }

    inline UPolynomial operator+(const UPolynomial& lhs, const UPolynomial& rhs) {
        return UPolynomial(lp_upolynomial_add(lhs.get(), rhs.get()));
    }
    inline UPolynomial operator-(const UPolynomial& lhs, const UPolynomial& rhs) {
        return UPolynomial(lp_upolynomial_sub(lhs.get(), rhs.get()));
    }
    inline UPolynomial operator*(const UPolynomial& lhs, const UPolynomial& rhs) {
        return UPolynomial(lp_upolynomial_mul(lhs.get(), rhs.get()));
    }

    std::vector<UPolynomial> square_free_factors(const UPolynomial& p, bool with_constant = false);

}
}
}
}

#endif