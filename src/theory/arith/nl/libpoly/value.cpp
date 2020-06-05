#include "value.h"

#include "util/rational.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

inline void value_deleter(lp_value_t* ptr) { lp_value_delete(ptr); }

Value::Value() : mValue(lp_value_new(LP_VALUE_NONE, nullptr), value_deleter) {}
Value::Value(const lp_value_t& val)
    : mValue(lp_value_new_copy(&val), value_deleter)
{
}
Value::Value(lp_value_t* ptr) : mValue(ptr, value_deleter) {}
Value::Value(const Value& val) : Value(lp_value_new_copy(val.get())) {}
Value::Value(Value&& val) : Value(val.release()) {}

Value::Value(const Integer& i): Value(lp_value_new(lp_value_type_t::LP_VALUE_INTEGER, i.get())) {
}
Value::Value(const RAN& r): Value(lp_value_new(lp_value_type_t::LP_VALUE_ALGEBRAIC, r.get())) {
}

Value& Value::operator=(const Value& v)
{
  mValue.reset(lp_value_new_copy(v.get()));
  return *this;
}
Value& Value::operator=(Value&& v)
{
  mValue = std::move(v.mValue);
  return *this;
}
Value& Value::operator=(lp_value_t* v)
{
  mValue.reset(v);
  return *this;
}

lp_value_t* Value::get() { return mValue.get(); }
const lp_value_t* Value::get() const { return mValue.get(); }
lp_value_t* Value::release() { return mValue.release(); }

Value Value::minus_infty()
{
  return Value(lp_value_new(LP_VALUE_MINUS_INFINITY, nullptr));
}
Value Value::plus_infty()
{
  return Value(lp_value_new(LP_VALUE_PLUS_INFINITY, nullptr));
}

Integer Value::as_integer() const {
  return Integer(get()->value.z);
}

RAN Value::as_ran() const {
  return RAN(get()->value.a);
}

std::ostream& operator<<(std::ostream& os, const Value& v)
{
  return os << lp_value_to_string(v.get());
}
bool operator==(const Value& lhs, const Value& rhs)
{
  return lp_value_cmp(lhs.get(), rhs.get()) == 0;
}
bool operator!=(const Value& lhs, const Value& rhs)
{
  if (lhs.get()->type == LP_VALUE_NONE) return true;
  if (rhs.get()->type == LP_VALUE_NONE) return true;
  return lp_value_cmp(lhs.get(), rhs.get()) != 0;
}
bool operator<(const Value& lhs, const Value& rhs)
{
  return lp_value_cmp(lhs.get(), rhs.get()) < 0;
}

Value sample_between(const lp_value_t* lhs,
                     bool l_strict,
                     const lp_value_t* rhs,
                     bool r_strict)
{
  Value res;
  lp_value_get_value_between(
      lhs, l_strict ? 1 : 0, rhs, r_strict ? 1 : 0, res.get());
  return res;
}

Value sample_between(const Value& lhs,
                     bool l_strict,
                     const Value& rhs,
                     bool r_strict)
{
  return sample_between(lhs.get(), l_strict, rhs.get(), r_strict);
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4
