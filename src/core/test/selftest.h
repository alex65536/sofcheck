#ifndef SOF_CORE_TEST_SELFTEST_INCLUDED
#define SOF_CORE_TEST_SELFTEST_INCLUDED

#include "core/board.h"

namespace SoFCore::Test {

// Checks that the board `b` is valid and updated. Otherwise this function panics.
void testBoardValid(const Board &b);

// Performs self-tests for board `b` (i.e. checks that many functions in `SoFCore` are
// implemented properly)
void runSelfTest(Board b);

}  // namespace SoFCore::Test

#endif  // SOF_CORE_TEST_SELFTEST_INCLUDED
