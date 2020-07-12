#ifndef SOF_GEN_COMMON_INCLUDED
#define SOF_GEN_COMMON_INCLUDED

#include <iostream>
#include <vector>

#include "core/types.h"

void printBitboard(std::ostream &out, SoFCore::bitboard_t val);

void printBitboardArray(std::ostream &out, const std::vector<SoFCore::bitboard_t> &array,
                        const char *name);

void printCoordArray(std::ostream &out, const std::vector<SoFCore::coord_t> &array,
                     const char *name);

#endif  // SOF_GEN_COMMON_INCLUDED
