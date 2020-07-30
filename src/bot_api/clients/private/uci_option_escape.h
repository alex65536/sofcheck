#ifndef SOF_BOT_API_CLIENTS_PRIVATE_UCI_OPTION_ESCAPE_INCLUDED
#define SOF_BOT_API_CLIENTS_PRIVATE_UCI_OPTION_ESCAPE_INCLUDED

#include <string>

namespace SoFBotApi::Clients::Private {

// Escapes UCI option name `name`. The UCI documentation requires that the option name cannot
// contain "name" and "value" tokens. `name` must be a list of tokens separated by a single space.
// If not, the function will re-tokenize the string, eliminating extra whitespaces.
std::string uciOptionNameEscape(const std::string &name);

// Unescapes UCI option name `name`
std::string uciOptionNameUnescape(const std::string &name);

// Escapes UCI enumeration item `item`. The UCI documentation requires that the enumeration item
// cannot contain "name", "value" and "val" tokens. `item` must be a list of tokens separated by a
// single space. If not, the function will re-tokenize the string, eliminating extra whitespaces.
std::string uciEnumNameEscape(const std::string &item);

// Unescapes UCI enumeration item `item`
std::string uciEnumNameUnescape(const std::string &item);

}  // namespace SoFBotApi::Clients::Private

#endif  // SOF_BOT_API_CLIENTS_PRIVATE_UCI_OPTION_ESCAPE_INCLUDED
