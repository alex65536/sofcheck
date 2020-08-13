#ifndef SOF_UTIL_OPERATORS_INCLUDED
#define SOF_UTIL_OPERATORS_INCLUDED

// Declares a single comparison operator `op` for enumeration type `type` with underlying integral
// type `base`
#define SOF_ENUM_COMPARE_OP(type, base, op)                       \
  inline constexpr bool operator op(const type a, const type b) { \
    return static_cast<base>(a) op static_cast<base>(b);          \
  }
// Declares all the comparison operators for enumeration type `type` with underlying integral type
// `base`
#define SOF_ENUM_COMPARE(type, base)  \
  SOF_ENUM_COMPARE_OP(type, base, ==) \
  SOF_ENUM_COMPARE_OP(type, base, !=) \
  SOF_ENUM_COMPARE_OP(type, base, <)  \
  SOF_ENUM_COMPARE_OP(type, base, <=) \
  SOF_ENUM_COMPARE_OP(type, base, >)  \
  SOF_ENUM_COMPARE_OP(type, base, >=)

#endif  // SOF_UTIL_OPERATORS_INCLUDED
