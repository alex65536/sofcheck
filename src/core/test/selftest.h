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

#ifndef SOF_CORE_TEST_SELFTEST_INCLUDED
#define SOF_CORE_TEST_SELFTEST_INCLUDED

namespace SoFCore {
struct Board;
}  // namespace SoFCore

namespace SoFCore::Test {

// Checks that the board `b` is valid and updated. Otherwise this function panics.
void testBoardValid(const Board &b);

// Performs self-tests for board `b` (i.e. checks that many functions in `SoFCore` are
// implemented properly)
void runSelfTest(Board b);

}  // namespace SoFCore::Test

#endif  // SOF_CORE_TEST_SELFTEST_INCLUDED
