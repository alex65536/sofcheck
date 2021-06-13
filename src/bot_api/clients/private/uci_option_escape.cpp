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

#include "bot_api/clients/private/uci_option_escape.h"

#include <algorithm>
#include <cstring>

#include "util/strutil.h"

namespace SoFBotApi::Clients::Private {

// Tokenizes the string `name`, transforms each token and rebuilds the string
template <typename Transform>
inline static std::string transformTokens(const std::string &name, Transform transform) {
  if (name.empty()) {
    return std::string();
  }
  std::string result;
  result.reserve(name.size() + 16);
  const char *str = name.c_str();
  str = SoFUtil::scanTokenStart(str);
  while (*str != '\0') {
    // Scan the next token
    const char *left = str;
    str = SoFUtil::scanTokenEnd(str);
    const char *right = str;
    str = SoFUtil::scanTokenStart(str);
    // Transform and add the token
    if (!result.empty()) {
      result += ' ';
    }
    result.append(transform(std::string(left, right)));
  }
  return result;
}

inline static bool isBadToken(const std::string &str) {
  // Find first characted not equal to "_"
  size_t pos = 0;
  while (pos < str.size() && str[pos] == '_') {
    ++pos;
  }
  static constexpr const char *BAD_WORDS[] = {"name", "value", "val"};
  return std::any_of(std::begin(BAD_WORDS), std::end(BAD_WORDS), [&](const char *badWord) {
    return std::equal(str.begin() + pos, str.end(), badWord, badWord + std::strlen(badWord));
  });
}

inline static std::string uciNameEscape(const std::string &name) {
  return transformTokens(name,
                         [](const std::string &str) { return (isBadToken(str) ? "_" : "") + str; });
}

inline static std::string uciNameUnescape(const std::string &name) {
  // NOTE: "value" escaped is "_value". When we try to unescape "_value", we get "value", when we
  // try to unescape "value", we also get "value". Maybe we should indicate that the string is
  // invalid in this case?
  return transformTokens(name, [](const std::string &str) {
    return (isBadToken(str) && str[0] == '_') ? str.substr(1) : str;
  });
}

std::string uciOptionNameEscape(const std::string &name) { return uciNameEscape(name); }

std::string uciOptionNameUnescape(const std::string &name) { return uciNameUnescape(name); }

std::string uciEnumNameEscape(const std::string &item) { return uciNameEscape(item); }

std::string uciEnumNameUnescape(const std::string &item) { return uciNameUnescape(item); }

}  // namespace SoFBotApi::Clients::Private
