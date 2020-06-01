
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__ASSIGNMENT_H
#define CVC4__THEORY__NLARITH__LIBPOLY__ASSIGNMENT_H

#include "utils.h"
#include "value.h"
#include "variable.h"

#include <poly/assignment.h>

#include <iostream>

namespace CVC4 {
namespace theory {
namespace nlarith {
namespace libpoly {

    /** A deleter for an std::unique_ptr holding a lp_assignment_t pointer */
    inline void assignment_deleter(lp_assignment_t* ptr) {
        lp_assignment_delete(ptr);
    }

    /**
     * Implements a wrapper for lp_assignment_t from libpoly.
     */
    class Assignment {
        friend std::ostream& operator<<(std::ostream& os, const Assignment& a);
        /** The actual assignment. */
        deleting_unique_ptr<lp_assignment_t> mAssignment;
    public:
        /** Construct an empty assignment. */
        Assignment(): mAssignment(lp_assignment_new(variable_db.get()), assignment_deleter) {}

        /** Get a non-const pointer to the internal lp_assignment_t. Handle with care! */
        lp_assignment_t* get() {
            return mAssignment.get();
        }
        /** Get a const pointer to the internal lp_assignment_t. */
        const lp_assignment_t* get() const {
            return mAssignment.get();
        }

        /** Assign var to the given value. */
        void set(const Variable& var, const Value& value) {
            lp_assignment_set_value(get(), var.get(), value.get());
        }
    };
    /** Stream the given Assignment to an output stream. */
    inline std::ostream& operator<<(std::ostream& os, const Assignment& a) {
        return os << lp_assignment_to_string(a.get());
    }

}
}
}
}

#endif