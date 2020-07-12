#ifndef SELFTEST_H_INCLUDED
#define SELFTEST_H_INCLUDED

#include "core/board.h"

namespace SoFCore::Test {

// All this functions throw std::logic error if something is wrong with the board

void testBoardValid(const Board &b);

void selfTest(Board b);

}  // namespace SoFCore::Test

#endif  // SELFTEST_H_INCLUDED
