// This file is part of SoFCheck
//
// Copyright (c) 2020-2021 Alexander Kernozhitsky and SoFCheck contributors
//
// SoFCheck is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SoFCheck is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SoFCheck.  If not, see <https://www.gnu.org/licenses/>.

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

// Dirty hack to prevent `clang-format` from splitting tokens
#define SOF_PRIVATE_OPERATORS_CONCAT(first, second) first##second

// Declares `op` operator for types `type1` and `type2` if `op=` operator is defined. This macro
// must be placed inside either the class `type1` or the class `type2`
#define SOF_PRIVATE_FROM_ASSIGNMENT_CLASS_OP(type1, type2, op, modifiers)    \
  friend modifiers type1 operator op(type1 a, const type2 &b) { /* NOLINT */ \
    return a SOF_PRIVATE_OPERATORS_CONCAT(op, =) b;             /* NOLINT */ \
  }
#define SOF_FROM_ASSIGNMENT_CLASS_OP_NOCONST(type1, type2, op) \
  SOF_PRIVATE_FROM_ASSIGNMENT_CLASS_OP(type1, type2, op, inline)
#define SOF_FROM_ASSIGNMENT_CLASS_OP(type1, type2, op) \
  SOF_PRIVATE_FROM_ASSIGNMENT_CLASS_OP(type1, type2, op, inline constexpr)

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
// type to that wrapper type. This macro must be located inside the class, and `op` must be a binary
// operator that modifies its first operand (like `+=`, `-=` and others)
#define SOF_PRIVATE_PROPAGATE_MUT_OP(type, field, op, modifiers) \
  modifiers type &operator op(const type &other) { /* NOLINT */  \
    field op other.field;                          /* NOLINT */  \
    return *this;                                                \
  }
#define SOF_PROPAGATE_MUT_OP_NOCONST(type, field, op) \
  SOF_PRIVATE_PROPAGATE_MUT_OP(type, field, op, inline)
#define SOF_PROPAGATE_MUT_OP(type, field, op) \
  SOF_PRIVATE_PROPAGATE_MUT_OP(type, field, op, inline constexpr)

// Same as `SOF_PROPAGATE_MUT_OP`, but the second operand is of another type `secondType`
#define SOF_PRIVATE_PROPAGATE_MUT_OP_EXT(type, field, op, secondType, modifiers) \
  modifiers type &operator op(const secondType &other) { /* NOLINT */            \
    field op other;                                      /* NOLINT */            \
    return *this;                                                                \
  }
#define SOF_PROPAGATE_MUT_OP_EXT_NOCONST(type, field, op, secondType) \
  SOF_PRIVATE_PROPAGATE_MUT_OP_EXT(type, field, op, secondType, inline)
#define SOF_PROPAGATE_MUT_OP_EXT(type, field, op, secondType) \
  SOF_PRIVATE_PROPAGATE_MUT_OP_EXT(type, field, op, secondType, inline constexpr)

// If `type` is a wrapper type that has a single field `field`, propagates `op` from the wrapped
// type to that wrapper type. This macro must be located inside the class, and `op` must be a unary
// operator (like `+`, `-` and others)
#define SOF_PRIVATE_PROPAGATE_UNARY_OP(type, field, op, modifiers) \
  modifiers type operator op() const { /* NOLINT */                \
    return type(op field);             /* NOLINT */                \
  }
#define SOF_PROPAGATE_UNARY_OP_NOCONST(type, field, op) \
  SOF_PRIVATE_PROPAGATE_UNARY_OP(type, field, op, inline)
#define SOF_PROPAGATE_UNARY_OP(type, field, op) \
  SOF_PRIVATE_PROPAGATE_UNARY_OP(type, field, op, inline constexpr)

// If `type` is a wrapper type that has a single field `field`, propagates `op` from the wrapped
// type to that wrapper type. This macro must be located inside the class, and `op` must be a
// comparison operator (like `==`, `!=` and others)
#define SOF_PROPAGATE_CMP_OP(type, field, op)                               \
  inline constexpr bool operator op(const type &other) const { /* NOLINT */ \
    return field op other.field;                               /* NOLINT */ \
  }

// Declares vector binary operators for vector type `type` with scalar type `scalar` (i.e. `+`, `-`,
// and `*`) based on `+=`, `-=` and `*=` implementation. This macro must be located inside the class
#define SOF_PRIVATE_VECTOR_OPS(type, scalar, modifiers)            \
  SOF_PRIVATE_FROM_ASSIGNMENT_CLASS_OP(type, type, +, modifiers)   \
  SOF_PRIVATE_FROM_ASSIGNMENT_CLASS_OP(type, type, -, modifiers)   \
  SOF_PRIVATE_FROM_ASSIGNMENT_CLASS_OP(type, scalar, *, modifiers) \
  friend modifiers type operator*(const scalar &a, type b) { return b *= a; } /* NOLINT */
#define SOF_VECTOR_OPS_NOCONST(type, scalar) SOF_PRIVATE_VECTOR_OPS(type, scalar, inline)
#define SOF_VECTOR_OPS(type, scalar) SOF_PRIVATE_VECTOR_OPS(type, scalar, inline constexpr)

// If `type` is a wrapper type that has a single field `field`, propagates vector operations. Type
// `scalar` is the scalar type involved in those operations. This macro must be located inside the
// class
#define SOF_PRIVATE_PROPAGATE_VECTOR_OPS(type, scalar, field, modifiers) \
  SOF_PROPAGATE_CMP_OP(type, field, ==)                                  \
  SOF_PROPAGATE_CMP_OP(type, field, !=)                                  \
  SOF_PRIVATE_PROPAGATE_MUT_OP(type, field, +=, modifiers)               \
  SOF_PRIVATE_PROPAGATE_MUT_OP(type, field, -=, modifiers)               \
  SOF_PRIVATE_PROPAGATE_MUT_OP_EXT(type, field, *=, scalar, modifiers)   \
  SOF_PRIVATE_PROPAGATE_UNARY_OP(type, field, -, modifiers)              \
  SOF_PRIVATE_PROPAGATE_UNARY_OP(type, field, +, modifiers)              \
  SOF_PRIVATE_VECTOR_OPS(type, scalar, modifiers)
#define SOF_PROPAGATE_VECTOR_OPS_NOCONST(type, scalar, field) \
  SOF_PRIVATE_PROPAGATE_VECTOR_OPS(type, scalar, field, inline)
#define SOF_PROPAGATE_VECTOR_OPS(type, scalar, field) \
  SOF_PRIVATE_PROPAGATE_VECTOR_OPS(type, scalar, field, inline constexpr)

#endif  // SOF_UTIL_OPERATORS_INCLUDED
