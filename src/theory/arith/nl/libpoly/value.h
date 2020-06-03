
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
  /** Create from a lp_value_t, creating a copy. */
  Value(const lp_value_t& val) : mValue(lp_value_new_copy(&val), value_deleter) {}
  /** Create from a lp_value_t pointer, claiming it's ownership. */
  Value(lp_value_t* ptr) : mValue(ptr, value_deleter) {}
  /** Copy from the given Value. */
  Value(const Value& val) : Value(lp_value_new_copy(val.get())) {}
  /** Move from the given Value. */
  Value(Value&& val) : Value(val.release()) {}
  
  /** Copy from the given Value. */
  Value& operator=(const Value& v)
  {
    mValue.reset(lp_value_new_copy(v.get()));
    return *this;
  }
  /** Move from the given Value. */
  Value& operator=(Value&& v)
  {
    mValue = std::move(v.mValue);
    return *this;
  }
  /** Assign from the given lp_value_t pointer, claiming it's ownership. */
  Value& operator=(lp_value_t* v)
  {
    mValue.reset(v);
    return *this;
  }

  /** Get a non-const pointer to the internal lp_value_t. Handle with care! */
  lp_value_t* get() { return mValue.get(); }
  /** Get a const pointer to the internal lp_value_t. */
  const lp_value_t* get() const { return mValue.get(); }
  /** Release the lp_value_t pointer. This yields ownership of the returned pointer. */
  lp_value_t* release() {
      return mValue.release();
  }

  /** Return -infty */
  static Value minus_infty() {
    return Value(lp_value_new(LP_VALUE_MINUS_INFINITY, nullptr));
  }
  /** Return +infty */
  static Value plus_infty() {
    return Value(lp_value_new(LP_VALUE_PLUS_INFINITY, nullptr));
  }
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
/** Compare values for disequality. */
inline bool operator!=(const Value& lhs, const Value& rhs)
{
  if (lhs.get()->type == LP_VALUE_NONE) return true;
  if (rhs.get()->type == LP_VALUE_NONE) return true;
  return lp_value_cmp(lhs.get(), rhs.get()) != 0;
}
/** Compare two values. */
inline bool operator<(const Value& lhs, const Value& rhs)
{
  return lp_value_cmp(lhs.get(), rhs.get()) < 0;
}

inline Value sample_between(const lp_value_t* lhs, bool l_strict, const lp_value_t* rhs, bool r_strict) {
  Value res;
  lp_value_get_value_between(lhs, l_strict ? 1 : 0, rhs, r_strict ? 1 : 0, res.get());
  return res;
}

inline Value sample_between(const Value& lhs, bool l_strict, const Value& rhs, bool r_strict) {
  return sample_between(lhs.get(), l_strict, rhs.get(), r_strict);
}


}  // namespace libpoly
}  // namespace nl
}  // namespace arith
}  // namespace theory
}  // namespace CVC4

#endif
