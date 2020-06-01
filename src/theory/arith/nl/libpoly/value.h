
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__VALUE_H
#define CVC4__THEORY__NLARITH__LIBPOLY__VALUE_H

#include "utils.h"

#include <poly/value.h>

#include <iostream>

namespace CVC4 {
namespace theory {
namespace nlarith {
namespace libpoly {

    inline void value_deleter(lp_value_t* ptr) {
        lp_value_delete(ptr);
    }

    class Value {
        friend std::ostream& operator<<(std::ostream& os, const Value& v);
        deleting_unique_ptr<lp_value_t> mValue;
    public:
        Value(): mValue(lp_value_new(LP_VALUE_NONE, nullptr), value_deleter) {}
        Value(lp_value_t* ptr): mValue(ptr, value_deleter) {}
        Value(const Value& val): Value(lp_value_new_copy(val.get())) {}

        lp_value_t* get() {
            return mValue.get();
        }
        const lp_value_t* get() const {
            return mValue.get();
        }
    };
    inline std::ostream& operator<<(std::ostream& os, const Value& v) {
        return os << lp_value_to_string(v.get());
    }
    inline bool operator==(const Value& lhs, const Value& rhs) {
        return lp_value_cmp(lhs.get(), rhs.get()) == 0;
    }

}
}
}
}

#endif