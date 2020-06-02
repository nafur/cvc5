
#ifndef CVC4__THEORY__NLARITH__LIBPOLY__VALUE_H
#define CVC4__THEORY__NLARITH__LIBPOLY__VALUE_H

#include <poly/value.h>

#include <iostream>

#include "utils.h"

namespace CVC4 {
namespace theory {
namespace arith {
namespace nl {
namespace libpoly {

/** A deleter for an std::unique_ptr holding a lp_value_t pointer */
inline void value_deleter(lp_value_t* ptr) { lp_value_delete(ptr); }

/**
 * Implements a wrapper for lp_value_t from libpoly.
 */
class Value
{
  friend std::ostream& operator<<(std::ostream& os, const Value& v);
  /** The actual value. */
  deleting_unique_ptr<lp_value_t> mValue;

 public:
  /** Construct a none value. */
  Value() : mValue(lp_value_new(LP_VALUE_NONE, nullptr), value_deleter) {}
  /** Create from a lp_value_t pointer, claiming it's ownership. */
  Value(lp_value_t* ptr) : mValue(ptr, value_deleter) {}
  /** Copy from the given Value. */
  Value(const Value& val) : Value(lp_value_new_copy(val.get())) {}

  /** Get a non-const pointer to the internal lp_value_t. Handle with care! */
  lp_value_t* get() { return mValue.get(); }
  /** Get a const pointer to the internal lp_value_t. */
  const lp_value_t* get() const { return mValue.get(); }
};

/** Stream the given Value to an output stream. */
inline std::ostream& operator<<(std::ostream& os, const Value& v)
{
  return os << lp_value_to_string(v.get());
}
/** Compare values for equality. */
inline bool operator==(const Value& lhs, const Value& rhs)
{
  return lp_value_cmp(lhs.get(), rhs.get()) == 0;
}

}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
