
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
namespace arith {
namespace nl {
namespace libpoly {

    /** A deleter for an std::unique_ptr holding a lp_upolynomial_t pointer */
    inline void upolynomial_deleter(lp_upolynomial_t* ptr) {
        lp_upolynomial_delete(ptr);
    }

    /**
     * Implements a wrapper for lp_upolynomial_t from libpoly.
     */
    class UPolynomial {
        friend std::ostream& operator<<(std::ostream& os, const UPolynomial& p);
        /** The actual univariate polynomial. */
        deleting_unique_ptr<lp_upolynomial_t> mPoly;
    public:
        /** Create from a lp_upolynomial_t pointer, claiming it's ownership. */
        UPolynomial(lp_upolynomial_t* poly): mPoly(poly, upolynomial_deleter) {}
        /** Create from a degree and a list of coefficients. */
        UPolynomial(std::size_t degree, const int* coeffs): mPoly(lp_upolynomial_construct_from_int(lp_Z, degree, coeffs), upolynomial_deleter) {}
        /** Create from a degree and a list of coefficients. */
        UPolynomial(std::size_t degree, const lp_integer_t* coeffs): mPoly(lp_upolynomial_construct(lp_Z, degree, coeffs), upolynomial_deleter) {}
        /** Copy from the given UPolynomial. */
        UPolynomial(const UPolynomial& poly): mPoly(lp_upolynomial_construct_copy(poly.get()), upolynomial_deleter) {}

        /** Get a non-const pointer to the internal lp_upolynomial_t. Handle with care! */
        lp_upolynomial_t* get() {
            return mPoly.get();
        }
        /** Get a const pointer to the internal lp_upolynomial_t. */
        const lp_upolynomial_t* get() const {
            return mPoly.get();
        }
        /** Release the lp_upolynomial_t pointer. This yields ownership of the returned pointer. */
        lp_upolynomial_t* release() {
            return mPoly.release();
        }
    };

    /** Stream the given UPolynomial to an output stream. */
    inline std::ostream& operator<<(std::ostream& os, const UPolynomial& p) {
        return os << lp_upolynomial_to_string(p.get());
    }

    /** Add two univariate polynomials. */
    inline UPolynomial operator+(const UPolynomial& lhs, const UPolynomial& rhs) {
        return UPolynomial(lp_upolynomial_add(lhs.get(), rhs.get()));
    }
    /** Subtract two univariate polynomials. */
    inline UPolynomial operator-(const UPolynomial& lhs, const UPolynomial& rhs) {
        return UPolynomial(lp_upolynomial_sub(lhs.get(), rhs.get()));
    }
    /** Multiply two univariate polynomials. */
    inline UPolynomial operator*(const UPolynomial& lhs, const UPolynomial& rhs) {
        return UPolynomial(lp_upolynomial_mul(lhs.get(), rhs.get()));
    }

    /** Obtain the degree of the given univariate polynomial. */
    inline std::size_t degree(const UPolynomial& p) {
        return lp_upolynomial_degree(p.get());
    }

    /**
     * Compute a square-free factorization of a univariate polynomial. 
     * Attention: this does not yield a full factorization!
     */
    std::vector<UPolynomial> square_free_factors(const UPolynomial& p, bool with_constant = false);

}
}
}
}
}

#endif
