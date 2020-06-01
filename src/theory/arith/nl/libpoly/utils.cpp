#include "utils.h"


namespace CVC4 {
namespace theory {
namespace nlarith {
namespace libpoly {

deleting_unique_ptr<lp_variable_db_t> variable_db = deleting_unique_ptr<lp_variable_db_t>(lp_variable_db_new(), [](lp_variable_db_t* ptr){ lp_variable_db_detach(ptr); });
deleting_unique_ptr<lp_variable_order_t> variable_order = deleting_unique_ptr<lp_variable_order_t>(lp_variable_order_new(), [](lp_variable_order_t* ptr){ lp_variable_order_detach(ptr); });
deleting_unique_ptr<lp_polynomial_context_t> polynomial_ctx = deleting_unique_ptr<lp_polynomial_context_t>(lp_polynomial_context_new(lp_Z, variable_db.get(), variable_order.get()), [](lp_polynomial_context_t* ptr){ lp_polynomial_context_detach(ptr); });

lp_variable_t fresh_variable(const char* name) {
    return lp_variable_db_new_variable(variable_db.get(), name);
}

}
}
}
}
