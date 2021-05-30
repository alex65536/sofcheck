#ifndef SOF_UTIL_OPERATORS_INCLUDED
#define SOF_UTIL_OPERATORS_INCLUDED

// Declares a single comparison operator `op` for enumeration type `type` with underlying integral
// type `base`
#define SOF_ENUM_COMPARE_OP(type, base, op)                                    \
  inline constexpr bool operator op(const type a, const type b) { /* NOLINT */ \
    return static_cast<base>(a) op static_cast<base>(b);          /* NOLINT */ \
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
#define SOF_ENUM_BITWISE_OP(type, base, op)                                              \
  inline constexpr type operator op(const type a, const type b) {           /* NOLINT */ \
    return static_cast<type>(static_cast<base>(a) op static_cast<base>(b)); /* NOLINT */ \
  }

// Declares `op=` operator for type `type` if `op` operator is defined
// NOLINTNEXTLINE(bugprone-macro-parentheses)
#define SOF_ASSIGNMENT_OP(type, op)                                           \
  inline constexpr type &operator op##=(type &a, const type b) { /* NOLINT */ \
    a = a op b;                                                  /* NOLINT */ \
    return a;                                                                 \
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
  inline constexpr type operator~(const type a) { return type::All ^ a; } /* NOLINT */

// If `type` is a wrapper type that has a single field `field`, propagates `op` from the wrapped
// type to that outer type. This macro must be located inside the class, and `op` must be a binary
// operator that modifies its first operand (like `+=`, `-=` and others)
#define SOF_PROPAGATE_MUT_OP(type, field, op)                          \
  inline constexpr type &operator op(const type &other) { /* NOLINT */ \
    field op other.field;                                 /* NOLINT */ \
    return *this;                                                      \
  }

// Same as `SOF_PROPAGATE_MUT_OP`, but the second operator is of another type `secondType`
#define SOF_PROPAGATE_MUT_OP_EXT(type, field, op, secondType)                \
  inline constexpr type &operator op(const secondType &other) { /* NOLINT */ \
    field op other;                                             /* NOLINT */ \
    return *this;                                                            \
  }

// If `type` is a wrapper type that has a single field `field`, propagates `op` from the wrapped
// type to that outer type. This macro must be located inside the class, and `op` must be a unary
// operator (like `+`, `-` and others)
#define SOF_PROPAGATE_UNARY_OP(type, field, op)            \
  inline constexpr type operator op() const { /* NOLINT */ \
    return type(op field);                    /* NOLINT */ \
  }

// If `type` is a wrapper type that has a single field `field`, propagates `op` from the wrapped
// type to that outer type. This macro must be located inside the class, and `op` must be a
// comparison operator (like `==`, `!=` and others)
#define SOF_PROPAGATE_CMP_OP(type, field, op)                               \
  inline constexpr bool operator op(const type &other) const { /* NOLINT */ \
    return field op other.field;                               /* NOLINT */ \
  }

// Declare vector binary operators form type `type` with scalar type `scalar` (i.e. `+`, `-`, and
// `*`) based on `+=`, `-=` and `*=` implementation. This macro must be located inside the class
#define SOF_VECTOR_OPS(type, scalar)                                                              \
  inline friend constexpr type operator+(type a, const type &b) { return a += b; }   /* NOLINT */ \
  inline friend constexpr type operator-(type a, const type &b) { return a -= b; }   /* NOLINT */ \
  inline friend constexpr type operator*(type a, const scalar &b) { return a *= b; } /* NOLINT */ \
  inline friend constexpr type operator*(const scalar &a, type b) { return b *= a; } /* NOLINT */

// If `type` is a wrapper type that has a single field `field`, propagates vector operations using
// scalar type `scalar`. This macro must be located inside the class
#define SOF_PROPAGATE_VECTOR_OPS(type, scalar, field) \
  SOF_PROPAGATE_CMP_OP(type, field, ==)               \
  SOF_PROPAGATE_CMP_OP(type, field, !=)               \
  SOF_PROPAGATE_MUT_OP(type, field, +=)               \
  SOF_PROPAGATE_MUT_OP(type, field, -=)               \
  SOF_PROPAGATE_MUT_OP_EXT(type, field, *=, scalar)   \
  SOF_PROPAGATE_UNARY_OP(type, field, -)              \
  SOF_PROPAGATE_UNARY_OP(type, field, +)              \
  SOF_VECTOR_OPS(type, scalar)

#endif  // SOF_UTIL_OPERATORS_INCLUDED
