#ifndef SELFTEST_H_INCLUDED
#define SELFTEST_H_INCLUDED

#include "core/board.h"

namespace SoFCore {
namespace Test {

// All this functions throw std::logic error if something is wrong with the board

void testBoardValid(const Board &b);

void selfTest(Board b);

}  // namespace Test
}  // namespace SoFCore

#endif  // SELFTEST_H_INCLUDED
