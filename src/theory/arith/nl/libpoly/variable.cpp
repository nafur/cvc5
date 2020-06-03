#include "variable.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

Variable::Variable() : mVariable(lp_variable_null) {}
Variable::Variable(lp_variable_t var) : mVariable(var) {}
Variable::Variable(const char* name)
    : mVariable(lp_variable_db_new_variable(variable_db.get(), name))
{
}

lp_variable_t Variable::get() const { return mVariable; }

std::ostream& operator<<(std::ostream& os, const Variable& v)
{
  return os << lp_variable_db_get_name(variable_db.get(), v.get());
}

bool operator==(const Variable& lhs, const Variable& rhs)
{
  return lhs.get() == rhs.get();
}
bool operator!=(const Variable& lhs, const Variable& rhs)
{
  return lhs.get() != rhs.get();
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
