#include "assignment.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/** A deleter for an std::unique_ptr holding a lp_assignment_t pointer */
inline void assignment_deleter(lp_assignment_t* ptr)
{
  lp_assignment_delete(ptr);
}

Assignment::Assignment()
    : mAssignment(lp_assignment_new(variable_db.get()), assignment_deleter)
{
}

lp_assignment_t* Assignment::get() { return mAssignment.get(); }
const lp_assignment_t* Assignment::get() const { return mAssignment.get(); }

void Assignment::set(const Variable& var, const Value& value)
{
  lp_assignment_set_value(get(), var.get(), value.get());
}
void Assignment::unset(const Variable& var)
{
  lp_assignment_set_value(get(), var.get(), nullptr);
}
bool Assignment::has(const Variable& var) const {
  return retrieve(var).get()->type != LP_VALUE_NONE;
}
Value Assignment::retrieve(const Variable& var) const {
  return *lp_assignment_get_value(get(), var.get());
}
void Assignment::clear()
{
  lp_assignment_destruct(mAssignment.get());
  lp_assignment_construct(mAssignment.get(), variable_db.get());
}

std::ostream& operator<<(std::ostream& os, const Assignment& a)
{
  return os << lp_assignment_to_string(a.get());
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4