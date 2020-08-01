#ifndef SOF_BOT_API_OPTIONS_INCLUDED
#define SOF_BOT_API_OPTIONS_INCLUDED

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "bot_api/api_base.h"
#include "util/no_copy_move.h"

namespace SoFBotApi {

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

// The observer for `OptionStorage` class. It receives the updates of the options.
class OptionObserver {
public:
  virtual ApiResult setBool(const std::string &key, bool value) = 0;
  virtual ApiResult setInt(const std::string &key, int64_t value) = 0;
  virtual ApiResult setEnum(const std::string &key, size_t index) = 0;
  virtual ApiResult setString(const std::string &key, const std::string &value) = 0;
  virtual ApiResult triggerAction(const std::string &key) = 0;
};

// Interface of the option holder. The option types are the same as used by UCI.
class Options : public SoFUtil::NoCopy {
public:
  // Returns true if the option `key` is present
  inline bool has(const std::string &key) const { return type(key) != OptionType::None; }

  // Returns the type of the option `key`. If `key` is not present, returns `OptionType::None`
  virtual OptionType type(const std::string &key) const = 0;

  // Lists all the options and their types
  virtual std::vector<std::pair<std::string, OptionType>> list() const = 0;

  // Returns pointer to the option `key` if this option is of type `Bool`. Otherwise returns
  // `nullptr`. The other `get...()` methods behave in the similar way.
  virtual const BoolOption *getBool(const std::string &key) const = 0;
  virtual const IntOption *getInt(const std::string &key) const = 0;
  virtual const EnumOption *getEnum(const std::string &key) const = 0;
  virtual const StringOption *getString(const std::string &key) const = 0;

  // Sets value `value` to the option `key` and returns the result of this operation.
  virtual ApiResult setBool(const std::string &key, bool value) = 0;
  virtual ApiResult setInt(const std::string &key, int64_t value) = 0;
  virtual ApiResult setEnum(const std::string &key, size_t index) = 0;
  virtual ApiResult setEnum(const std::string &key, const std::string &value) = 0;
  virtual ApiResult setString(const std::string &key, const std::string &value) = 0;

  // Calls the observer and triggers the option `key` of type `Action`. Options of such type cannot
  // be set, they can only be triggered.
  virtual ApiResult triggerAction(const std::string &key) = 0;
};

// The holder of engine client options. This class is not thread-safe
class OptionStorage final : public Options {
public:
  OptionType type(const std::string &key) const override;

  std::vector<std::pair<std::string, OptionType>> list() const override;

  const BoolOption *getBool(const std::string &key) const override { return getT<BoolOption>(key); }
  const IntOption *getInt(const std::string &key) const override { return getT<IntOption>(key); }
  const EnumOption *getEnum(const std::string &key) const override { return getT<EnumOption>(key); }
  const StringOption *getString(const std::string &key) const override {
    return getT<StringOption>(key);
  }

  // The methods below work in the following way:
  // - first, validate the option
  // - then call the observer (if it's present)
  // - if the observer is absent or returned `ApiResult::Ok`, really set the option
  ApiResult setBool(const std::string &key, bool value) override;
  ApiResult setInt(const std::string &key, int64_t value) override;
  ApiResult setEnum(const std::string &key, size_t index) override;
  ApiResult setEnum(const std::string &key, const std::string &value) override;
  ApiResult setString(const std::string &key, const std::string &value) override;

  // The algorithm of this method is as follows:
  // - validate that the option is present and is of type `Action`
  // - then call the observer if it's present
  ApiResult triggerAction(const std::string &key) override;

private:
  friend class OptionBuilder;

  inline explicit OptionStorage(OptionObserver *observer) : observer_(observer) {}

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

// Option holder with a lock
template <typename Mutex>
class SyncOptionStorage final : public Options {
public:
  OptionType type(const std::string &key) const override {
    std::lock_guard guard(mutex_);
    return options_.type(key);
  }

  std::vector<std::pair<std::string, OptionType>> list() const override {
    std::lock_guard guard(mutex_);
    return options_.list();
  }

  const BoolOption *getBool(const std::string &key) const override {
    std::lock_guard guard(mutex_);
    return options_.getBool(key);
  }

  const IntOption *getInt(const std::string &key) const override {
    std::lock_guard guard(mutex_);
    return options_.getInt(key);
  }

  const EnumOption *getEnum(const std::string &key) const override {
    std::lock_guard guard(mutex_);
    return options_.getEnum(key);
  }

  const StringOption *getString(const std::string &key) const override {
    std::lock_guard guard(mutex_);
    return options_.getString(key);
  }

  ApiResult setBool(const std::string &key, bool value) override {
    std::lock_guard guard(mutex_);
    return options_.setBool(key, value);
  }

  ApiResult setInt(const std::string &key, int64_t value) override {
    std::lock_guard guard(mutex_);
    return options_.setInt(key, value);
  }

  ApiResult setEnum(const std::string &key, size_t index) override {
    std::lock_guard guard(mutex_);
    return options_.setEnum(key, index);
  }

  ApiResult setEnum(const std::string &key, const std::string &value) override {
    std::lock_guard guard(mutex_);
    return options_.setEnum(key, value);
  }

  ApiResult setString(const std::string &key, const std::string &value) override {
    std::lock_guard guard(mutex_);
    return options_.setString(key, value);
  }

  ApiResult triggerAction(const std::string &key) override {
    std::lock_guard guard(mutex_);
    return options_.triggerAction(key);
  }

private:
  friend class OptionBuilder;

  SyncOptionStorage(OptionStorage options, Mutex &mutex)
      : options_(std::move(options)), mutex_(mutex) {}

  OptionStorage options_;
  Mutex &mutex_;
};

// Builds instance of `Option`
class OptionBuilder : public SoFUtil::NoCopyMove {
public:
  // Creates the builder and sets the observer for the new `Options` instance
  inline explicit OptionBuilder(OptionObserver *observer = nullptr) : options_(observer) {}

  // Adds options to the `Options` instance being built. If invalid options are supplied or the same
  // option is added twice, these functions panic.
  //
  // All these methods return `*this`.
  //
  // Also please note that the option names must be valid, i.e. `isOptionNameValid()` must return
  // `true`. This rule also applies to enumeration items, i.e. all the strings in `items` must be
  // valid option names.

  OptionBuilder &addBool(const std::string &key, bool value) noexcept;
  OptionBuilder &addEnum(const std::string &key, std::vector<std::string> items,
                         size_t index) noexcept;
  OptionBuilder &addInt(const std::string &key, int64_t min, int64_t value, int64_t max) noexcept;
  OptionBuilder &addString(const std::string &key, std::string value) noexcept;
  OptionBuilder &addAction(const std::string &key) noexcept;

  // The methods below move the built instance of `Option` from the class. Do not use the builder
  // after this method is called.
  inline OptionStorage options() { return std::move(options_); }

  template <typename Mutex>
  inline SyncOptionStorage<Mutex> optionsSync(Mutex &mutex) {
    return SyncOptionStorage<Mutex>(std::move(options_), mutex);
  }

private:
  // Helper method for `add...()` methods
  template <typename T>
  OptionBuilder &addT(const std::string &key, T t) noexcept;

  OptionStorage options_;
};

// Checks if the option name is valid. This contains checks that:
// - all the characters are from ASCII 32 to ASCII 126
// - no two spaces in a row
// - the name must not start or end with space
// - the string is not empty
bool isOptionNameValid(const std::string &s);

}  // namespace SoFBotApi

#endif  // SOF_BOT_API_OPTIONS_INCLUDED
