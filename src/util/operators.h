#ifndef SOF_UTIL_OPERATORS_INCLUDED
#define SOF_UTIL_OPERATORS_INCLUDED

// Declares a single comparison operator `op` for enumeration type `type` with underlying integral
// type `base`
#define SOF_ENUM_COMPARE_OP(type, base, op)                       \
  inline constexpr bool operator op(const type a, const type b) { \
    return static_cast<base>(a) op static_cast<base>(b);          \
  }

// Declares `==` and `!=` operators for enumeration type `type` with underlying integral type `base`
#define SOF_ENUM_EQUAL(type, base)    \
  SOF_ENUM_COMPARE_OP(type, base, ==) \
  SOF_ENUM_COMPARE_OP(type, base, !=)

// Declares all the comparison operators for enumeration type `type` with underlying integral type
// `base`
#define SOF_ENUM_COMPARE(type, base)  \
  SOF_ENUM_COMPARE_OP(type, base, ==) \
  SOF_ENUM_COMPARE_OP(type, base, !=) \
  SOF_ENUM_COMPARE_OP(type, base, <)  \
  SOF_ENUM_COMPARE_OP(type, base, <=) \
  SOF_ENUM_COMPARE_OP(type, base, >)  \
  SOF_ENUM_COMPARE_OP(type, base, >=)

// Declares a single bitwise operator `op` for enumeration type `type` with underlying integral
// type `base`
#define SOF_ENUM_BITWISE_OP(type, base, op)                                 \
  inline constexpr type operator op(const type a, const type b) {           \
    return static_cast<type>(static_cast<base>(a) op static_cast<base>(b)); \
  }

// Declares `op=` operator for type `type` if `op` operator is defined
#define SOF_ASSIGNMENT_OP(type, op)                              \
  inline constexpr type &operator op##=(type &a, const type b) { \
    a = a op b;                                                  \
    return a;                                                    \
  }

// Declares all the bitwise operators for enumeration type `type` with underlying integral type
// `base`. Note that `type` must contain `type::All` member which must contain all possible bits set
// and which will be used to define bitwise not operator.
#define SOF_ENUM_BITWISE(type, base)              \
  SOF_ENUM_BITWISE_OP(type, base, &)              \
  SOF_ENUM_BITWISE_OP(type, base, |)              \
  SOF_ENUM_BITWISE_OP(type, base, ^)              \
  SOF_ASSIGNMENT_OP(type, &)                      \
  SOF_ASSIGNMENT_OP(type, |)                      \
  SOF_ASSIGNMENT_OP(type, ^)                      \
  /* Bitwise `~` operator is treated specially */ \
  inline constexpr type operator~(const type a) { return type::All ^ a; }

#endif  // SOF_UTIL_OPERATORS_INCLUDED
