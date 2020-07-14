#ifndef BITUTIL_H_INCLUDED
#define BITUTIL_H_INCLUDED

#ifdef __GNUG__
    inline int popcount(int64_t b) { return __builtin_popcountll(b); }
    #define HAS_POPCOUNT
#endif // __GNUG__
#ifdef _MSC_VER
    #include <intrin.h>
    inline int popcount(int64_t b) { return __popcnt64(b); }
    #define HAS_POPCOUNT
#endif // _MSC_VER
#ifndef HAS_POPCOUNT
    #include <limits>
    #include <bitset>
    inline int popcount(int64_t b) { return std::bitset<std::numeric_limits<BITBOARD>::digits>(b).count(); }
    #define HAS_POPCOUNT
#endif // HAS_POPCOUNT
#ifndef HAS_POPCOUNT
    #error popcount function is not implemented for this compiler!!!
#endif // HAS_POPCOUNT

#endif // BITUTIL_H_INCLUDED
