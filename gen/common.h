#ifndef SOF_GEN_COMMON_INCLUDED
#define SOF_GEN_COMMON_INCLUDED

#include <cstdint>
#include <iostream>
#include <vector>

#include "core/types.h"

void printBitboard(std::ostream &out, SoFCore::bitboard_t val);

void printBitboardArray(std::ostream &out, const std::vector<SoFCore::bitboard_t> &array,
                        const char *name, size_t indent = 0);

void printCoordArray(std::ostream &out, const std::vector<SoFCore::coord_t> &array,
                     const char *name, size_t indent = 0);

void printIntArray(std::ostream &out, const std::vector<int32_t> &array, const char *name,
                   const char *typeName = "int", size_t indent = 0);

#endif  // SOF_GEN_COMMON_INCLUDED
