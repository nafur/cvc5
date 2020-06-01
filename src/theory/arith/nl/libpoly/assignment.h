
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

    inline void assignment_deleter(lp_assignment_t* ptr) {
        lp_assignment_delete(ptr);
    }

    class Assignment {
        friend std::ostream& operator<<(std::ostream& os, const Assignment& a);
        deleting_unique_ptr<lp_assignment_t> mAssignment;
    public:
        Assignment(): mAssignment(lp_assignment_new(variable_db.get()), assignment_deleter) {}

        lp_assignment_t* get() {
            return mAssignment.get();
        }
        const lp_assignment_t* get() const {
            return mAssignment.get();
        }

        void set(const Variable& var, const Value& value) {
            lp_assignment_set_value(get(), var.get(), value.get());
        }
    };
    inline std::ostream& operator<<(std::ostream& os, const Assignment& a) {
        return os << lp_assignment_to_string(a.get());
    }


}
}
}
}

#endif