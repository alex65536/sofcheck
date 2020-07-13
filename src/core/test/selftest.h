#ifndef SOF_CORE_TEST_SELFTEST_INCLUDED
#define SOF_CORE_TEST_SELFTEST_INCLUDED

#include "core/board.h"

namespace SoFCore::Test {

// All this functions panic if something is wrong with the board

void testBoardValid(const Board &b);

void selfTest(Board b);

}  // namespace SoFCore::Test

#endif  // SOF_CORE_TEST_SELFTEST_INCLUDED
