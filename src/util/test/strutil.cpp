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

TEST(SoFUtil, WordWrap_1) {
  const std::string src =
      "The string that needs  to be wrapped, even if wordisverylong.  Be careful with      multi "
      "spaces!   ";
  const auto result = SoFUtil::wordWrap(std::string_view(src), 10);
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

TEST(SoFUtil, WordWrap_2) {
  const std::string src = "  First indent must be kept.";
  const auto result = SoFUtil::wordWrap(std::string_view(src), 20);
  ASSERT_EQ(result.size(), 2U);
  EXPECT_EQ(result[0], "  First indent must");
  EXPECT_EQ(result[1], "be kept.");
}

TEST(SoFUtil, WordWrap_3) {
  const std::string src =
      "  The quick brown fox jumps over the lazy dog.\nNext line?\n\nReally next line!  \n  "
      "Another line...        ";
  const auto result = SoFUtil::wordWrap(std::string_view(src), 20);
  ASSERT_EQ(result.size(), 7U);
  EXPECT_EQ(result[0], "  The quick brown");
  EXPECT_EQ(result[1], "fox jumps over the");
  EXPECT_EQ(result[2], "lazy dog.");
  EXPECT_EQ(result[3], "Next line?");
  EXPECT_EQ(result[4], "");
  EXPECT_EQ(result[5], "Really next line!");
  EXPECT_EQ(result[6], "  Another line...");
}

TEST(SoFUtil, WordWrap_4) {
  const std::string src = "    word    longword    a a    b    c   d";
  const auto result = SoFUtil::wordWrap(std::string_view(src), 4);
  ASSERT_EQ(result.size(), 7U);
  EXPECT_EQ(result[0], "word");
  EXPECT_EQ(result[1], "long");
  EXPECT_EQ(result[2], "word");
  EXPECT_EQ(result[3], "a a");
  EXPECT_EQ(result[4], "b");
  EXPECT_EQ(result[5], "c");
  EXPECT_EQ(result[6], "d");
}

TEST(SoFUtil, WordWrap_5) {
  const std::string src = "      long start";
  const auto result = SoFUtil::wordWrap(std::string_view(src), 6);
  ASSERT_EQ(result.size(), 2U);
  EXPECT_EQ(result[0], "long");
  EXPECT_EQ(result[1], "start");
}

TEST(SoFUtil, WordWrap_6) {
  const std::string src = "  shorter start";
  const auto result = SoFUtil::wordWrap(std::string_view(src), 7);
  ASSERT_EQ(result.size(), 3U);
  EXPECT_EQ(result[0], "  short");
  EXPECT_EQ(result[1], "er");
  EXPECT_EQ(result[2], "start");
}

TEST(SoFUtil, WordWrap_InfiniteWidth) {
  constexpr size_t INFINITE_WIDTH = static_cast<size_t>(-1);

  const std::string src1 =
      "  This is some text.  \nAs the width is infinite, it will be only splitted by newlines.\n";
  const std::string src2 = "Text1\nText2   long     spaces\n\nText 3";
  const auto result1 = SoFUtil::wordWrap(src1, INFINITE_WIDTH);
  const auto result2 = SoFUtil::wordWrap(src2, INFINITE_WIDTH);

  ASSERT_EQ(result1.size(), 2U);
  EXPECT_EQ(result1[0], "  This is some text.");
  EXPECT_EQ(result1[1], "As the width is infinite, it will be only splitted by newlines.");

  ASSERT_EQ(result2.size(), 4U);
  EXPECT_EQ(result2[0], "Text1");
  EXPECT_EQ(result2[1], "Text2   long     spaces");
  EXPECT_EQ(result2[2], "");
  EXPECT_EQ(result2[3], "Text 3");
}

TEST(SoFUtil, WordWrap_EmptyLines) {
  const std::string src1 = "";
  const std::string src2 = "    ";
  const std::string src3 = "    \n\n   \n";
  const std::string src4 = "\n";
  const auto result1 = SoFUtil::wordWrap(src1, 100);
  const auto result2 = SoFUtil::wordWrap(src2, 100);
  const auto result3 = SoFUtil::wordWrap(src3, 100);
  const auto result4 = SoFUtil::wordWrap(src4, 100);

  ASSERT_EQ(result1.size(), 1U);
  EXPECT_TRUE(result1[0].empty());

  ASSERT_EQ(result2.size(), 1U);
  EXPECT_TRUE(result2[0].empty());

  ASSERT_EQ(result3.size(), 3U);
  EXPECT_TRUE(result3[0].empty());
  EXPECT_TRUE(result3[1].empty());
  EXPECT_TRUE(result3[2].empty());

  ASSERT_EQ(result4.size(), 1U);
  EXPECT_TRUE(result4[0].empty());
}
