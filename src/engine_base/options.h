#ifndef SOF_ENGINE_BASE_OPTIONS_INCLUDED
#define SOF_ENGINE_BASE_OPTIONS_INCLUDED

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "engine_base/api_base.h"
#include "util/no_copy_move.h"

namespace SoFEngineBase {

enum class OptionType { Bool = 0, Int = 1, String = 2, Enum = 3, Action = 4, None = 255 };

struct BoolOption {
  bool value;
};

struct IntOption {
  int64_t minValue;
  int64_t value;
  int64_t maxValue;
};

struct StringOption {
  std::string value;
};

struct EnumOption {
  std::vector<std::string> items;
  size_t index;
};

struct ActionOption {};

// The observer for `Options` class. It receives the updates of the options.
class OptionObserver {
public:
  virtual ApiResult setBool(const std::string &key, bool value) = 0;
  virtual ApiResult setInt(const std::string &key, int64_t value) = 0;
  virtual ApiResult setEnum(const std::string &key, size_t index) = 0;
  virtual ApiResult setString(const std::string &key, const std::string &value) = 0;
  virtual ApiResult triggerAction(const std::string &key) = 0;
};

// The holder of engine client options. The option types are the same as used by UCI.
class Options : public SoFUtil::NoCopy {
public:
  // Returns true if the option `key` is present
  inline bool has(const std::string &key) { return values_.count(key); }

  // Returns the type of the option `key`. If `key` is not present, returns `OptionType::None`
  inline OptionType type(const std::string &key) const {
    auto iter = values_.find(key);
    if (iter == values_.end()) {
      return OptionType::None;
    }
    return static_cast<OptionType>(iter->second.index());
  }

  // Lists all the options and their types
  std::vector<std::pair<std::string, OptionType>> list() const;

  // Returns pointer to the option `key` if it's of type `Bool`. Otherwise returns `nullptr`. The
  // other `get...()` methods behave in the similar way.
  inline const BoolOption *getBool(const std::string &key) const { return getT<BoolOption>(key); }
  inline const IntOption *getInt(const std::string &key) const { return getT<IntOption>(key); }
  inline const EnumOption *getEnum(const std::string &key) const { return getT<EnumOption>(key); }
  inline const StringOption *getString(const std::string &key) const {
    return getT<StringOption>(key);
  }

  // Sets value to the option `key` and return result appropriately. The algorithm is as follows:
  // - first, validate the option
  // - then call the observer (if it's present)
  // - if the observer is absent or returned `ApiResult::Ok`, really set the option
  ApiResult setBool(const std::string &key, bool value) noexcept;
  ApiResult setInt(const std::string &key, int64_t value) noexcept;
  ApiResult setEnum(const std::string &key, size_t index) noexcept;
  ApiResult setEnum(const std::string &key, const std::string &value) noexcept;
  ApiResult setString(const std::string &key, const std::string &value) noexcept;

  // Calls the observer and triggers the option `key` of type `Action`. Such options cannot be set,
  // they can only be triggered. The algorithm is as follows:
  // - validate that the option is present and of type `Action`
  // - then call the observer if it's present
  ApiResult triggerAction(const std::string &key) noexcept;

private:
  friend class OptionBuilder;

  inline explicit Options(OptionObserver *observer) : observer_(observer) {}

  // Helper method for `asBool`, `asInt`, `asString` and `asEnum`
  template <typename T>
  inline const T *getT(const std::string &key) const {
    auto iter = values_.find(key);
    if (iter == values_.end()) {
      return nullptr;
    }
    return std::get_if<T>(&iter->second);
  }

  // Helper method for `setBool`, `setInt`, `setString` and `setEnum`
  template <typename T>
  inline T *getMutT(const std::string &key) {
    auto iter = values_.find(key);
    if (iter == values_.end()) {
      return nullptr;
    }
    return std::get_if<T>(&iter->second);
  }

  // Helper method for `setBool`, `setInt`, `setString` and `setEnum`
  template <typename T, typename Val, typename Validator, typename Observer>
  ApiResult setT(const std::string &key, const Val &value, Validator validate,
                 Observer observe) noexcept;

  using OptValue = std::variant<BoolOption, IntOption, StringOption, EnumOption, ActionOption>;

  OptionObserver *observer_;
  std::unordered_map<std::string, OptValue> values_;
};

// Builds instance of `Option`
class OptionBuilder : public SoFUtil::NoCopyMove {
public:
  // Creates the builder and sets the observer for the new `Options` instance
  inline explicit OptionBuilder(OptionObserver *observer = nullptr) : options_(observer) {}

  // Adds options to the `Options` instance being built. If invalid options supplies or the same
  // option is added twice, the functions panic.
  //
  // All these methods return `*this`.
  //
  // Also please note that the option names names must be valid, i.e. `isOptionNameValid()` must
  // return `true`. This rule also applies to enumeration items, i.e. all the strings in `items`
  // must be valid option names.

  OptionBuilder &addBool(const std::string &key, bool value) noexcept;
  OptionBuilder &addEnum(const std::string &key, std::vector<std::string> items,
                         size_t index) noexcept;
  OptionBuilder &addInt(const std::string &key, int64_t min, int64_t value, int64_t max) noexcept;
  OptionBuilder &addString(const std::string &key, std::string value) noexcept;
  OptionBuilder &addAction(const std::string &key) noexcept;

  // Moves the built instance of `Option` from the class. Do not use the builder after this
  // method is called.
  inline Options options() { return std::move(options_); }

private:
  // Helper method for `add...()` methods
  template <typename T>
  OptionBuilder &addT(const std::string &key, T t) noexcept;

  Options options_;
};

// Checks if the option name is valid. This contains checks that:
// - all the characters are from ASCII 32 to ASCII 126
// - no two spaces in a row
// - the name must not start or end with space
// - the string is not empty
bool isOptionNameValid(const std::string &s);

}  // namespace SoFEngineBase

#endif  // SOF_ENGINE_BASE_OPTIONS_INCLUDED
