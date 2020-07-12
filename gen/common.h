#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <iostream>
#include <vector>

#include "core/types.h"

void printBitboard(std::ostream &out, SoFCore::bitboard_t val);

void printBitboardArray(std::ostream &out, const std::vector<SoFCore::bitboard_t> &array,
                        const char *name);

void printCoordArray(std::ostream &out, const std::vector<SoFCore::coord_t> &array,
                     const char *name);

#endif  // COMMON_H_INCLUDED
