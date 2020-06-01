
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__UTILS_H
#define CVC4__THEORY__NLARITH__LIBPOLY__UTILS_H

#include <poly/polynomial_context.h>
#include <poly/variable_db.h>
#include <poly/variable_order.h>

#include <functional>
#include <iostream>
#include <memory>

namespace CVC4 {
namespace theory {
namespace nlarith {
namespace libpoly {

    template<typename T>
    using deleting_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

    extern deleting_unique_ptr<lp_variable_db_t> variable_db;
    extern deleting_unique_ptr<lp_variable_order_t> variable_order;
    extern deleting_unique_ptr<lp_polynomial_context_t> polynomial_ctx;

}
}
}
}

#endif