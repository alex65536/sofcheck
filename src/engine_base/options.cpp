#include "engine_base/options.h"

#include <algorithm>
#include <type_traits>

#include "util/misc.h"

namespace SoFEngineBase {

using SoFUtil::panic;

std::vector<std::pair<std::string, OptionType>> Options::list() const {
  std::vector<std::pair<std::string, OptionType>> result;
  result.reserve(values_.size());
  for (const auto &[key, value] : values_) {
    result.emplace_back(key, static_cast<OptionType>(value.index()));
  }
  return result;
}

template <typename T, typename Val, typename Validator, typename Observer>
ApiResult Options::setT(const std::string &key, const Val &value, Validator validate,
                        Observer observe) noexcept {
  T *option = getMutT<T>(key);
  if (!option || !validate(*option)) {
    return ApiResult::InvalidArgument;
  }
  if (observer_) {
    const ApiResult res = observe();
    if (res != ApiResult::Ok) {
      return res;
    }
  }
  if constexpr (std::is_same<T, EnumOption>::value) {
    option->index = value;
  } else {
    option->value = value;
  }
  return ApiResult::Ok;
}

ApiResult Options::setBool(const std::string &key, const bool value) noexcept {
  return setT<BoolOption>(
      key, value, [&](const BoolOption &) { return true; },
      [&]() { return observer_->setBool(key, value); });
}

ApiResult Options::setEnum(const std::string &key, size_t index) noexcept {
  return setT<EnumOption>(
      key, index, [&](const EnumOption &o) { return index < o.items.size(); },
      [&]() { return observer_->setEnum(key, index); });
}

ApiResult Options::setEnum(const std::string &key, const std::string &value) noexcept {
  EnumOption *option = getMutT<EnumOption>(key);
  if (!option) {
    return ApiResult::InvalidArgument;
  }
  const std::vector<std::string> &items = option->items;
  auto iter = std::find(items.begin(), items.end(), value);
  if (iter == items.end()) {
    return ApiResult::InvalidArgument;
  }
  const size_t index = iter - items.begin();
  if (observer_) {
    const ApiResult res = observer_->setEnum(key, index);
    if (res != ApiResult::Ok) {
      return res;
    }
  }
  option->index = index;
  return ApiResult::Ok;
}

ApiResult Options::setInt(const std::string &key, int64_t value) noexcept {
  return setT<IntOption>(
      key, value, [&](const IntOption &o) { return o.minValue <= value && value <= o.maxValue; },
      [&]() { return observer_->setInt(key, value); });
}

ApiResult Options::setString(const std::string &key, const std::string &value) noexcept {
  return setT<StringOption>(
      key, value, [&](const StringOption &) { return true; },
      [&]() { return observer_->setString(key, value); });
}

ApiResult Options::triggerAction(const std::string &key) noexcept {
  ActionOption *option = getMutT<ActionOption>(key);
  if (!option) {
    return ApiResult::InvalidArgument;
  }
  if (observer_) {
    return observer_->triggerAction(key);
  }
  return ApiResult::Ok;
}

OptionBuilder &OptionBuilder::addBool(const std::string &key, const bool value) noexcept {
  return addT(key, BoolOption{value});
}

template <typename T>
inline static bool hasRepetitions(std::vector<T> vec) {
  std::sort(vec.begin(), vec.end());
  for (size_t i = 1; i < vec.size(); ++i) {
    if (vec[i - 1] == vec[i]) {
      return true;
    }
  }
  return false;
}

OptionBuilder &OptionBuilder::addEnum(const std::string &key, std::vector<std::string> items,
                                      size_t index) noexcept {
  if (index >= items.size()) {
    panic("Invalid EnumOption given for the key \"" + key + "\"");
  }
  // Check if all the items have correct names
  for (const std::string &item : items) {
    if (!isOptionNameValid(item)) {
      panic("Attempt to add item with invalid name \"" + item + "\"");
    }
  }
  // Check if some items repeat
  if (hasRepetitions(items)) {
    panic("Some enumeration items repeat");
  }
  return addT(key, EnumOption{std::move(items), index});
}

OptionBuilder &OptionBuilder::addInt(const std::string &key, const int64_t min, const int64_t value,
                                     const int64_t max) noexcept {
  if (value < min || value > max) {
    panic("Invalid IntOption given for the key \"" + key + "\"");
  }
  return addT(key, IntOption{min, value, max});
}

OptionBuilder &OptionBuilder::addString(const std::string &key, std::string value) noexcept {
  return addT(key, StringOption{std::move(value)});
}

OptionBuilder &OptionBuilder::addAction(const std::string &key) noexcept {
  return addT(key, ActionOption{});
}

template <typename T>
OptionBuilder &OptionBuilder::addT(const std::string &key, T t) noexcept {
  if (!isOptionNameValid(key)) {
    panic("Attempt to add invalid key \"" + key + "\"");
  }
  const bool added = options_.values_.try_emplace(key, std::in_place_type<T>, std::move(t)).second;
  if (!added) {
    panic("Attempt to add key \"" + key + "\" twice");
  }
  return *this;
}

bool isOptionNameValid(const std::string &s) {
  if (s.empty() || s[0] == ' ' || s.back() == ' ') {
    return false;
  }
  for (char c : s) {
    const int charCode = c;
    if (charCode < 32 || charCode > 126) {
      return false;
    }
  }
  for (size_t i = 1; i < s.size(); ++i) {
    if (s[i - 1] == ' ' && s[i] == ' ') {
      return false;
    }
  }
  return true;
}

}  // namespace SoFEngineBase
