
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__INTEGER_H
#define CVC4__THEORY__NLARITH__LIBPOLY__INTEGER_H

#include "utils.h"

#include <poly/integer.h>

#include <iostream>

namespace CVC4 {
namespace theory {
namespace nlarith {
namespace libpoly {

    /**
     * Implements a wrapper for lp_integer_t from libpoly.
     */
    class Integer {
        friend std::ostream& operator<<(std::ostream& os, const Integer& i);
        /** The actual integer. */
        lp_integer_t mInt;
    public:
        /** Construct a zero integer. */
        Integer() {
            lp_integer_construct(&mInt);
        }
        /** Construct from the given integer. */
        Integer(long i) {
            lp_integer_construct_from_int(lp_Z, &mInt, i);
        }
        /** Copy from the given Integer. */
        Integer(const Integer& i) {
            lp_integer_construct_copy(lp_Z, &mInt, i.get());
        }
        /** Custom destructor. */
        ~Integer() {
            lp_integer_destruct(&mInt);
        }
        /** Assign from the given Integer. */
        Integer& operator=(Integer i) {
            std::swap(mInt, i.mInt);
            return *this;
        }

        /** Get a non-const pointer to the internal lp_integer_t. Handle with care! */
        lp_integer_t* get() {
            return &mInt;
        }
        /** Get a const pointer to the internal lp_integer_t. */
        const lp_integer_t* get() const {
            return &mInt;
        }
    };
    /** Stream the given Integer to an output stream. */
    inline std::ostream& operator<<(std::ostream& os, const Integer& i) {
        return os << lp_integer_to_string(i.get());
    }

    /** Unary negation for an Integer. */
    inline Integer operator-(const Integer& i) {
        Integer res;
        lp_integer_neg(lp_Z, res.get(), i.get());
        return res;
    }

}
}
}
}

#endif