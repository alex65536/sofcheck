// This file is part of SoFCheck
//
// Copyright (c) 2021 Alexander Kernozhitsky and SoFCheck contributors
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

#include "util/strutil.h"

#include <gtest/gtest.h>

TEST(SoFUtil, WordWrap1) {
  std::string src =
      "The string that needs  to be wrapped, even if wordisverylong.  Be careful with      multi "
      "spaces!   ";
  auto result = SoFUtil::wordWrap(std::string_view(src), 10);
  ASSERT_EQ(result.size(), 11U);
  EXPECT_EQ(result[0], "The string");
  EXPECT_EQ(result[1], "that needs");
  EXPECT_EQ(result[2], "to be");
  EXPECT_EQ(result[3], "wrapped,");
  EXPECT_EQ(result[4], "even if");
  EXPECT_EQ(result[5], "wordisvery");
  EXPECT_EQ(result[6], "long.  Be");
  EXPECT_EQ(result[7], "careful");
  EXPECT_EQ(result[8], "with");
  EXPECT_EQ(result[9], "multi");
  EXPECT_EQ(result[10], "spaces!");
}

TEST(SoFUtil, WordWrap2) {
  std::string src = "  First indent must be kept.";
  auto result = SoFUtil::wordWrap(std::string_view(src), 20);
  ASSERT_EQ(result.size(), 2U);
  EXPECT_EQ(result[0], "  First indent must");
  EXPECT_EQ(result[1], "be kept.");
}

TEST(SoFUtil, WordWrap3) {
  std::string src =
      "  The quick brown fox jumps over the lazy dog.\nNext line?\n\nReally next line!  \n  "
      "Another line...        ";
  auto result = SoFUtil::wordWrap(std::string_view(src), 20);
  ASSERT_EQ(result.size(), 7U);
  EXPECT_EQ(result[0], "  The quick brown");
  EXPECT_EQ(result[1], "fox jumps over the");
  EXPECT_EQ(result[2], "lazy dog.");
  EXPECT_EQ(result[3], "Next line?");
  EXPECT_EQ(result[4], "");
  EXPECT_EQ(result[5], "Really next line!");
  EXPECT_EQ(result[6], "  Another line...");
}

TEST(SoFUtil, WordWrap4) {
  std::string src = "    word    longword    a a    b    c";
  auto result = SoFUtil::wordWrap(std::string_view(src), 4);
  ASSERT_EQ(result.size(), 6U);
  EXPECT_EQ(result[0], "word");
  EXPECT_EQ(result[1], "long");
  EXPECT_EQ(result[2], "word");
  EXPECT_EQ(result[3], "a a");
  EXPECT_EQ(result[4], "b");
  EXPECT_EQ(result[5], "c");
}

TEST(SoFUtil, WordWrapEmptyLiness) {
  std::string src1 = "";
  std::string src2 = "    ";
  std::string src3 = "    \n\n   \n";
  std::string src4 = "\n";
  auto result1 = SoFUtil::wordWrap(src1, 100);
  auto result2 = SoFUtil::wordWrap(src2, 100);
  auto result3 = SoFUtil::wordWrap(src3, 100);
  auto result4 = SoFUtil::wordWrap(src4, 100);

  ASSERT_EQ(result1.size(), 1);
  EXPECT_TRUE(result1[0].empty());

  ASSERT_EQ(result2.size(), 1);
  EXPECT_TRUE(result2[0].empty());

  ASSERT_EQ(result3.size(), 3);
  EXPECT_TRUE(result3[0].empty());
  EXPECT_TRUE(result3[1].empty());
  EXPECT_TRUE(result3[2].empty());

  ASSERT_EQ(result4.size(), 1);
  EXPECT_TRUE(result4[0].empty());
}
