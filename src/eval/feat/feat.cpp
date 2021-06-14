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

#include "eval/feat/feat.h"

#include <json/json.h>

#include <algorithm>
#include <iomanip>
#include <memory>
#include <type_traits>
#include <unordered_set>
#include <utility>

#include "util/formatter.h"
#include "util/strutil.h"

namespace SoFEval::Feat {

using Private::BundleGroupImpl;
using SoFUtil::Err;  // NOLINT: clang-tidy thinks it's unused by some reason
using SoFUtil::Ok;   // NOLINT: clang-tidy thinks it's unused by some reason
using SoFUtil::SourceFormatter;

namespace Private {

// Helper struct to implement operations for a group of bundles without writing too much boilerplate
// code. All the methods in this struct just call `T::iterate()` to list all the sub-bundles of
// `obj` and delegate operations to these sub-bundles
struct BundleGroupImpl {
  template <typename T>
  static void save(const T &obj, Json::Value &json) {
    T::iterate(obj, [&](const auto &bundle, const char *name) { bundle.save(json[name]); });
  }

  template <typename T>
  static void apply(T &obj, const WeightVec &weights) {
    T::iterate(obj, [&](auto &bundle, const char *) { bundle.apply(weights); });
  }

  template <typename T>
  static void extract(const T &obj, WeightVec &weights) {
    T::iterate(obj, [&](const auto &bundle, const char *) { bundle.extract(weights); });
  }

  template <typename T>
  static size_t count(const T &obj) {
    size_t result = 0;
    T::iterate(obj, [&](const auto &bundle, const char *) { result += bundle.count(); });
    return result;
  }

  template <typename T>
  static std::vector<Name> names(const T &obj) {
    std::vector<Name> result;
    result.reserve(obj.count());
    T::iterate(obj, [&](const auto &bundle, const char *) {
      const auto names = bundle.names();
      result.insert(result.end(), names.begin(), names.end());
    });
    return result;
  }
};

}  // namespace Private

LoadResult<SingleBundle> SingleBundle::load(const Name &name, const Json::Value &json) {
  if (!json.isInt()) {
    return Err(LoadError{name.name + " must be int"});
  }
  return Ok(SingleBundle(name, json.asInt()));
}

void SingleBundle::save(Json::Value &json) const { json = value_; }

void SingleBundle::print(SourceFormatter &fmt) const { fmt.stream() << value_; }

LoadResult<ArrayBundle> ArrayBundle::load(const Name &name, const Json::Value &json) {
  if (!json.isArray()) {
    return Err(LoadError{name.name + " must be array"});
  }
  std::vector<weight_t> weights(json.size());
  for (Json::ArrayIndex idx = 0; idx < json.size(); ++idx) {
    const Json::Value &item = json[idx];
    if (!item.isInt()) {
      return Err(LoadError{name.name + "." + std::to_string(idx) + " must be int"});
    }
    weights[idx] = item.asInt();
  }
  return Ok(ArrayBundle(name, std::move(weights)));
}

void ArrayBundle::save(Json::Value &json) const {
  json = Json::Value(Json::arrayValue);
  json.resize(static_cast<Json::ArrayIndex>(values_.size()));
  for (Json::ArrayIndex idx = 0; idx < json.size(); ++idx) {
    json[idx] = values_[idx];
  }
}

void ArrayBundle::print(SourceFormatter &fmt) const {
  fmt.stream() << "[";
  for (size_t idx = 0; idx < values_.size(); ++idx) {
    if (idx != 0) {
      fmt.stream() << ", ";
    }
    fmt.stream() << values_[idx];
  }
  fmt.stream() << "]";
}

void ArrayBundle::apply(const WeightVec &weights) {
  for (size_t idx = 0; idx < values_.size(); ++idx) {
    values_[idx] = weights[idx + name_.offset];
  }
}

void ArrayBundle::extract(WeightVec &weights) const {
  for (size_t idx = 0; idx < values_.size(); ++idx) {
    weights[idx + name_.offset] = values_[idx];
  }
}

std::vector<Name> ArrayBundle::names() const {
  std::vector<Name> result(values_.size());
  for (size_t idx = 0; idx < values_.size(); ++idx) {
    result[idx] = Name{name_.offset + idx, name_.name + "." + std::to_string(idx)};
  }
  return result;
}

template <typename ThisType, typename Callback>
void PsqBundle::iterate(ThisType &obj, Callback callback) {
  static_assert(std::is_same_v<PsqBundle, std::remove_cv_t<ThisType>>);

  callback(obj.pieceCosts_, "cost");
  for (size_t idx = 0; idx < PIECE_COUNT; ++idx) {
    callback(obj.tables_[idx], PIECE_NAMES[idx]);
  }
  callback(obj.endKingTable_, "king_end");
}

LoadResult<PsqBundle> PsqBundle::load(const Name &name, const Json::Value &json) {
  if (!json.isObject() || json.get("type", Json::Value("")) != "psq") {
    return Err(LoadError{name.name + " must be object with type = psq"});
  }

  PsqBundle bundle(name);

  size_t curOffset = name.offset;

  auto doLoad = [&](ArrayBundle &sub, const char *subName,
                    size_t len) -> LoadResult<std::monostate> {
    if (!json.isMember(subName)) {
      return Err(LoadError{name.name + "." + subName + " doesn\'t exist"});
    }
    const Name curName{curOffset, name.name + "." + subName};
    SOF_TRY_ASSIGN(sub, ArrayBundle::load(curName, json[subName]));
    if (sub.count() != len) {
      return Err(
          LoadError{name.name + "." + subName + " must contain " + std::to_string(len) + " items"});
    }
    curOffset += sub.count();
    return Ok(std::monostate{});
  };

  SOF_TRY_CONSUME(doLoad(bundle.pieceCosts_, "cost", PIECE_COUNT));
  for (size_t idx = 0; idx < PIECE_COUNT; ++idx) {
    SOF_TRY_CONSUME(doLoad(bundle.tables_[idx], PIECE_NAMES[idx], 64));
  }
  SOF_TRY_CONSUME(doLoad(bundle.endKingTable_, "king_end", 64));

  return Ok(std::move(bundle));
}

void PsqBundle::save(Json::Value &json) const {
  json = Json::Value(Json::objectValue);
  json["type"] = "psq";
  BundleGroupImpl::save(*this, json);
}

void PsqBundle::print(SourceFormatter &fmt) const {
  fmt.stream() << "{\n";
  fmt.indent(1);
  fmt.line() << R"TEXT("type": "psq",)TEXT";

  fmt.lineStart() << R"TEXT("cost": )TEXT";
  pieceCosts_.print(fmt);
  fmt.stream() << ",\n";

  auto printBoard = [&](const ArrayBundle &board, const char *name) {
    const std::vector<weight_t> &values = board.values();
    SOF_ASSERT(values.size() == 64);

    size_t columnSizes[8] = {};
    for (size_t idx = 0; idx < 64; ++idx) {
      const size_t col = idx & 7;
      columnSizes[col] = std::max(columnSizes[col], SoFUtil::intStrLen(values[idx]));
    }

    fmt.line() << "\"" << name << "\": [";
    fmt.indent(1);
    for (size_t x = 0; x < 8; ++x) {
      auto line = fmt.line();
      for (size_t y = 0; y < 8; ++y) {
        line << std::setw(columnSizes[y]) << values[(x << 3) | y];
        if (x != 7 || y != 7) {
          line << ",";
        }
        if (y != 7) {
          line << " ";
        }
      }
    }
    fmt.outdent(1);
    fmt.lineStart() << "]";
  };

  for (size_t idx = 0; idx < PIECE_COUNT; ++idx) {
    printBoard(tables_[idx], PIECE_NAMES[idx]);
    fmt.stream() << ",\n";
  }
  printBoard(endKingTable_, "king_end");
  fmt.stream() << "\n";

  fmt.outdent(1);
  fmt.lineStart() << "}";
}

void PsqBundle::apply(const WeightVec &weights) { BundleGroupImpl::apply(*this, weights); }
void PsqBundle::extract(WeightVec &weights) const { BundleGroupImpl::extract(*this, weights); }
std::vector<Name> PsqBundle::names() const { return BundleGroupImpl::names(*this); }
size_t PsqBundle::count() const { return BundleGroupImpl::count(*this); }

LoadResult<Bundle> Bundle::load(const Name &name, const Json::Value &json) {
#define D_TRY_LOAD(type)                               \
  {                                                    \
    return type::load(name, json).map([](type res) {   \
      return Bundle(VariantItemTag{}, std::move(res)); \
    });                                                \
  }

  if (json.isInt()) {
    D_TRY_LOAD(SingleBundle);
  }
  if (json.isArray()) {
    D_TRY_LOAD(ArrayBundle);
  }
  if (json.isObject()) {
    const Json::Value type = json.get("type", Json::Value(""));
    if (type == "psq") {
      D_TRY_LOAD(PsqBundle);
    }
  }

  return Err(LoadError{name.name + " has unknown type"});
#undef D_TRY_LOAD
}

template <typename ThisType, typename Callback>
void Features::iterate(ThisType &obj, Callback callback) {
  static_assert(std::is_same_v<Features, std::remove_cv_t<ThisType>>);
  for (auto &bundle : obj.bundles_) {
    callback(bundle, bundle.name().name.c_str());
  }
}

LoadResult<Features> Features::load(const Json::Value &json) {
  if (!json.isArray()) {
    return Err(LoadError{"Feature JSON must be array"});
  }

  std::unordered_set<std::string> added;
  std::vector<Bundle> bundles(json.size());
  size_t counter = 0;

  for (Json::ArrayIndex idx = 0; idx < json.size(); ++idx) {
    const Json::Value &item = json[idx];
    if (!item.isObject() || item.size() != 1) {
      return Err(
          LoadError{R"TEXT(Each item of feature JSON must have the form {"key": "value"})TEXT"});
    }
    const std::string key = item.getMemberNames()[0];
    if (!added.insert(key).second) {
      return Err(LoadError{"Bundle " + key + " is present at least twice"});
    }
    SOF_TRY_ASSIGN(bundles[idx], Bundle::load(Name{counter, key}, item[key]));
    counter += bundles[idx].count();
  }

  return Ok(Features(std::move(bundles), counter));
}

LoadResult<Features> Features::load(std::istream &in) {
  Json::Value json;
  Json::CharReaderBuilder builder;
  std::string errs;
  if (!Json::parseFromStream(builder, in, &json, &errs)) {
    return Err(LoadError{"JSON parse error: " + errs});
  }
  return load(json);
}

void Features::save(Json::Value &json) const {
  json = Json::Value(Json::arrayValue);
  for (const auto &bundle : bundles_) {
    Json::Value &item = json.append(Json::Value(Json::objectValue));
    bundle.save(item[bundle.name().name]);
  }
}

void Features::print(SoFUtil::SourceFormatter &fmt) const {
  fmt.line() << "[";
  fmt.indent(1);
  for (size_t idx = 0; idx < bundles_.size(); ++idx) {
    const auto &bundle = bundles_[idx];
    fmt.lineStart() << "{\"" << bundle.name().name << "\": ";
    bundle.print(fmt);
    fmt.stream() << "}";
    if (idx + 1 != bundles_.size()) {
      fmt.stream() << ",";
    }
    fmt.stream() << "\n";
  }
  fmt.outdent(1);
  fmt.line() << "]";
}

void Features::print(std::ostream &out) const {
  SourceFormatter fmt(out, 4);
  print(fmt);
}

void Features::apply(const WeightVec &weights) {
  SOF_ASSERT(weights.size() == count());
  BundleGroupImpl::apply(*this, weights);
}

WeightVec Features::extract() const {
  WeightVec result(count());
  BundleGroupImpl::extract(*this, result);
  return result;
}

std::vector<Name> Features::names() const {
  std::vector<Name> result = BundleGroupImpl::names(*this);

  // Ensure that the features arrive in the sequential order
  SOF_ASSERT(result.size() == count());
  for (size_t idx = 0; idx < result.size(); ++idx) {
    SOF_ASSERT(result[idx].offset == idx);
  }

  return result;
}

}  // namespace SoFEval::Feat
