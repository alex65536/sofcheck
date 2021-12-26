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

#include "util/strutil.h"

#include <algorithm>

#include "util/math.h"

namespace SoFUtil {

const char *scanTokenEnd(const char *str) {
  while (*str != '\0' && !isSpace(*str)) {
    ++str;
  }
  return str;
}

const char *scanTokenStart(const char *str) {
  while (isSpace(*str)) {
    ++str;
  }
  return str;
}

std::vector<std::string_view> split(const char *str) {
  std::vector<std::string_view> result;
  str = scanTokenStart(str);
  while (*str != '\0') {
    const char *next = scanTokenEnd(str);
    result.emplace_back(str, next - str);
    str = scanTokenStart(next);
  }
  return result;
}

bool startsWith(const std::string_view &s, const std::string_view &t) {
  return s.size() >= t.size() && s.substr(0, t.size()) == t;
}

std::string sanitizeEol(std::string str) {
  for (char &c : str) {
    if (c < ' ' && c != '\t') {
      c = ' ';
    }
  }
  return str;
}

std::string_view trim(const std::string_view &str) {
  size_t left = 0;
  size_t right = str.size();
  while (left < str.size() && isSpace(str[left])) {
    ++left;
  }
  if (left == right) {
    return std::string_view();
  }
  while (right != 0 && isSpace(str[right - 1])) {
    --right;
  }
  return str.substr(left, right - left);
}

std::string trimmed(const std::string &str) {
  std::string result(trim(str));
  return result;
}

void asciiToLower(std::string &s) {
  for (char &ch : s) {
    ch = asciiToLower(ch);
  }
}

void asciiToUpper(std::string &s) {
  for (char &ch : s) {
    ch = asciiToUpper(ch);
  }
}

void replace(std::string &s, const char src, const char dst) {
  for (char &ch : s) {
    ch = (ch == src) ? dst : ch;
  }
}

size_t intStrLen(const int64_t value) {
  return uintStrLen(static_cast<uint64_t>(std::abs(value))) + ((value < 0) ? 1 : 0);
}

size_t uintStrLen(const uint64_t value) { return (value == 0) ? 1 : 1 + SoFUtil::log10(value); }

std::vector<std::string_view> wordWrap(const std::string_view &s, const size_t width) {
  std::vector<std::string_view> result;

  if (s.empty()) {
    result.push_back(s);
    return result;
  }

  auto processLine = [&result, &width](std::string_view line) {
    // Trim right
    size_t rightSpacePos = line.size();
    while (rightSpacePos != 0 && line[rightSpacePos - 1] == ' ') {
      --rightSpacePos;
    }
    line = line.substr(0, rightSpacePos);
    if (line.empty()) {
      result.push_back(line);
      return;
    }

    // Trim left
    size_t leftSpacePos = 0;
    while (leftSpacePos < line.size() && line[leftSpacePos] == ' ') {
      ++leftSpacePos;
    }

    // We try to keep the spaces in the start of the line. However, if their amount exceeds the line
    // width, we remove them all
    size_t lineStartPos = (leftSpacePos >= width) ? leftSpacePos : 0;
    size_t lineEndPos = lineStartPos;
    bool isFirstWord = true;

    while (lineStartPos < line.size()) {
      // Seek the next word to add to the current line. First, scan the spaces that separate this
      // word from the previous one
      size_t wordStartPos = lineEndPos;
      while (wordStartPos < line.size() && line[wordStartPos] == ' ') {
        ++wordStartPos;
      }
      if (wordStartPos == line.size() || wordStartPos - lineStartPos >= width) {
        // This is either the end of the string, or we will overflow when adding spaces to the
        // current line. In both cases, it's OK to finish the current line and just drop the spaces.
        // (Note that in the first case there must be no spaces to drop here, as we removed trailing
        // spaces earlier)
        result.push_back(line.substr(lineStartPos, lineEndPos - lineStartPos));
        lineStartPos = wordStartPos;
        lineEndPos = wordStartPos;
        isFirstWord = true;
        continue;
      }

      // Then, find the end of the word which we want to add
      size_t wordEndPos = wordStartPos;
      while (wordEndPos < line.size() && wordEndPos - lineStartPos <= width &&
             line[wordEndPos] != ' ') {
        ++wordEndPos;
      }
      if (wordEndPos - lineStartPos > width) {
        // The word is too long, we need to break the line without including this word. Still, if
        // there's no more words in the line, we must include it. In this case we just add the first
        // `width` letters of this word to the line and don't respect wrapping by spaces
        if (isFirstWord) {
          // The only word in the line
          result.push_back(line.substr(lineStartPos, width));
          lineStartPos += width;
          lineEndPos = lineStartPos;
          isFirstWord = true;
          continue;
        }
        result.push_back(line.substr(lineStartPos, lineEndPos - lineStartPos));
        lineStartPos = wordStartPos;
        lineEndPos = wordStartPos;
        isFirstWord = true;
        continue;
      }

      // The word can be safely added
      lineEndPos = wordEndPos;
      isFirstWord = false;
    }
  };

  // First, we split the input string by line endings. Then we call `processLine`, which will split
  // the source line into multiple lines, so they will not be larger than `width`
  for (size_t right = 0; right < s.size();) {
    const size_t left = right;
    while (right < s.size() && s[right] != '\n') {
      ++right;
    }
    processLine(s.substr(left, right - left));
    if (right < s.size()) {
      ++right;
    }
  }

  return result;
}

}  // namespace SoFUtil
