
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__UTILS_H
#define CVC4__THEORY__NLARITH__LIBPOLY__UTILS_H

#include "util/utility.h"

#include <poly/polynomial_context.h>
#include <poly/variable_db.h>
#include <poly/variable_order.h>

#include <functional>
#include <iostream>
#include <memory>
#include <vector>

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/** Generic type alias for a unique_ptr with a deleter function. */
template <typename T>
using deleting_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

/** A central variable database. */
extern deleting_unique_ptr<lp_variable_db_t> variable_db;
/** A central variable ordering. */
extern deleting_unique_ptr<lp_variable_order_t> variable_order;
/** A central polynomial context. */
extern deleting_unique_ptr<lp_polynomial_context_t> polynomial_ctx;

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    container_to_stream(os, v);
    return os;
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
