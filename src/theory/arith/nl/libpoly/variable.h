
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__VARIABLE_H
#define CVC4__THEORY__NLARITH__LIBPOLY__VARIABLE_H

#include "utils.h"

#include <poly/variable_db.h>

#include <iostream>

namespace CVC4 {
namespace theory {
namespace nlarith {
namespace libpoly {

    class Variable {
        friend std::ostream& operator<<(std::ostream& os, const Variable& v);
        lp_variable_t mVariable;
    public:
        Variable(const char* name): mVariable(lp_variable_db_new_variable(variable_db.get(), name)) {}

        lp_variable_t get() const {
            return mVariable;
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const Variable& v) {
        return os << lp_variable_db_get_name(variable_db.get(), v.get());
    }

}
}
}
}

#endif