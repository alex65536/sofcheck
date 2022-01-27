// This file is part of SoFCheck
//
// Copyright (c) 2021-2022 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_EVAL_FEAT_FEAT_INCLUDED
#define SOF_EVAL_FEAT_FEAT_INCLUDED

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "util/result.h"

namespace Json {
class Value;
}  // namespace Json

namespace SoFUtil {
class SourceFormatter;
}  // namespace SoFUtil

namespace SoFEval::Feat {

namespace Private {
struct BundleGroupImpl;
}  // namespace Private

using weight_t = int32_t;

// Feature name. It includes the symbolic name and the position in the feature vector
struct Name {
  size_t offset;
  std::string name;
};

// Error indicating that the features failed to load from JSON
struct LoadError {
  std::string description;
};

template <typename T>
using LoadResult = SoFUtil::Result<T, LoadError>;

using WeightVec = std::vector<weight_t>;

// A bundle that contains a single feature. To see the description of the methods, refer to `Bundle`
// struct
class SingleBundle {
public:
  static LoadResult<SingleBundle> load(const Name &name, const Json::Value &json);
  void save(Json::Value &json) const;
  void print(SoFUtil::SourceFormatter &fmt) const;
  void apply(const WeightVec &weights) { value_ = weights[name_.offset]; }
  void extract(WeightVec &weights) const { weights[name_.offset] = value_; }
  std::vector<Name> names() const { return {name_}; }
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  size_t count() const { return 1; }
  const Name &name() const { return name_; }

  // Returns the stored weight
  weight_t value() const { return value_; }

  SingleBundle() = default;

private:
  SingleBundle(Name name, const weight_t value) : name_(std::move(name)), value_(value) {}

  Name name_;
  weight_t value_;
};

// A bundle that represents an array of features. To see the description of the methods, refer to
// `Bundle` struct
class ArrayBundle {
public:
  static LoadResult<ArrayBundle> load(const Name &name, const Json::Value &json);
  void save(Json::Value &json) const;
  void print(SoFUtil::SourceFormatter &fmt) const;
  void apply(const WeightVec &weights);
  void extract(WeightVec &weights) const;
  std::vector<Name> names() const;
  size_t count() const { return values_.size(); }
  const Name &name() const { return name_; }

  // Returns the weigths in the array
  const std::vector<weight_t> &values() const { return values_; }

  ArrayBundle() = default;

private:
  ArrayBundle(Name name, std::vector<weight_t> values)
      : name_(std::move(name)), values_(std::move(values)) {}

  Name name_;
  std::vector<weight_t> values_;
};

// A bundle that represents a piece-square table. To see the description of the methods, refer to
// `Bundle` struct
class PsqBundle {
public:
  static LoadResult<PsqBundle> load(const Name &name, const Json::Value &json);
  void save(Json::Value &json) const;
  void print(SoFUtil::SourceFormatter &fmt) const;
  void apply(const WeightVec &weights);
  void extract(WeightVec &weights) const;
  std::vector<Name> names() const;
  size_t count() const;
  const Name &name() const { return name_; }

  // Returns the bundle that corresponds to costs of pieces
  const ArrayBundle &pieceCosts() const { return pieceCosts_; }

  // Returns the bundle that corresponds to bonuses for each piece for occupying some position
  const ArrayBundle &table(size_t idx) const { return tables_[idx]; }

  // Returns the bundle that corresponds to bonuses for king at the end of the game
  const ArrayBundle &endKingTable() const { return endKingTable_; }

  PsqBundle() = default;

private:
  static constexpr size_t PIECE_COUNT = 6;
  static constexpr const char *PIECE_NAMES[6] = {"pawn",   "king", "knight",
                                                 "bishop", "rook", "queen"};

  template <typename ThisType, typename Callback>
  static void iterate(ThisType &obj, Callback callback);

  friend struct Private::BundleGroupImpl;

  explicit PsqBundle(Name name) : name_(std::move(name)) {}

  Name name_;
  ArrayBundle pieceCosts_;
  std::array<ArrayBundle, PIECE_COUNT> tables_;
  ArrayBundle endKingTable_;
};

// A bundle that represents king's interaction with pawns. To see the description of the methods,
// refer to `Bundle` struct
class KingPawnBundle {
public:
  static LoadResult<KingPawnBundle> load(const Name &name, const Json::Value &json);
  void save(Json::Value &json) const;
  void print(SoFUtil::SourceFormatter &fmt) const;
  void apply(const WeightVec &weights);
  void extract(WeightVec &weights) const;
  std::vector<Name> names() const;
  size_t count() const;
  const Name &name() const { return name_; }

  // Returns the bundle that corresponds to pawn shield
  const ArrayBundle &shield() const { return shield_; }

  // Returns the bundle that corresponds to pawn storm
  const ArrayBundle &storm() const { return storm_; }

  KingPawnBundle() = default;

private:
  template <typename ThisType, typename Callback>
  static void iterate(ThisType &obj, Callback callback);

  friend struct Private::BundleGroupImpl;

  explicit KingPawnBundle(Name name) : name_(std::move(name)) {}

  Name name_;
  ArrayBundle shield_;
  ArrayBundle storm_;
};

// A container for all the bundles described above
class Bundle {
public:
  // Loads the bundle from `json` and assigns it the name `name`
  static LoadResult<Bundle> load(const Name &name, const Json::Value &json);

  // Prints the bundle to `fmt`. The result will be formatted better than just printing
  // `Json::Value` obtained by calling `save()`
  void print(SoFUtil::SourceFormatter &fmt) const {
    std::visit([&](const auto &x) { x.print(fmt); }, inner_);
  }

  // Stores the bundle in `json`
  void save(Json::Value &json) const {
    std::visit([&](const auto &x) { x.save(json); }, inner_);
  }

  // Applies the weights from the vector `weights` into the bundle. The given vector must be large
  // enough to contain all the required weights, otherwise the behavior is undefined
  void apply(const WeightVec &weights) {
    std::visit([&](auto &x) { x.apply(weights); }, inner_);
  }

  // Extracts the weights from the bundle into the vector `weights`. The given vector must be large
  // enough to store all the required weights, otherwise the behavior is undefined
  void extract(WeightVec &weights) const {
    std::visit([&](const auto &x) { x.extract(weights); }, inner_);
  }

  // Returns the names of all the features in the bundle, in sequential order (i. e.
  // `names()[i].offset` == `name().offset + i`)
  std::vector<Name> names() const {
    return std::visit([&](const auto &x) { return x.names(); }, inner_);
  }

  // Returns the amount of the features in the bundle
  size_t count() const {
    return std::visit([&](const auto &x) { return x.count(); }, inner_);
  }

  // Returns the name of the bundle
  const Name &name() const {
    return std::visit([&](const auto &x) -> const Name & { return x.name(); }, inner_);
  }

#define D_AS_BUNDLE(type)                                                 \
  type##Bundle *as##type() { return std::get_if<type##Bundle>(&inner_); } \
  const type##Bundle *as##type() const { return std::get_if<type##Bundle>(&inner_); }

  // The following methods (`asSingle`, `asArray` and `asPsq`) allow to cast `Bundle` to a
  // corresponding subtype
  D_AS_BUNDLE(Single)
  D_AS_BUNDLE(Array)
  D_AS_BUNDLE(Psq)
  D_AS_BUNDLE(KingPawn)

#undef D_AS_BUNDLE

  // Default constuctor. Do not call any of the methods when the object is in default-constructed
  // state
  Bundle() = default;

private:
  struct VariantItemTag {};

  template <typename T>
  Bundle(VariantItemTag, T value) : inner_(std::move(value)) {}

  std::variant<SingleBundle, ArrayBundle, PsqBundle, KingPawnBundle> inner_;
};

// The container for all the features
class Features {
public:
  // Loads the features from `json`
  static LoadResult<Features> load(const Json::Value &json);

  // Load features from the stream `in`
  static LoadResult<Features> load(std::istream &in);

  // Stores the features in `json`
  void save(Json::Value &json) const;

  // Prints the features to `fmt`. The result will be formatted better than just printing
  // `Json::Value` obtained by calling `save()`
  void print(SoFUtil::SourceFormatter &fmt) const;

  // Prints the features to the stream `out`
  void print(std::ostream &out) const;

  // Applies the weights from the vector `weights`. Note that `weights.size() == count()` must hold,
  // otherwise the function will panic
  void apply(const WeightVec &weights);

  // Gathers the weights from the features into the vector
  WeightVec extract() const;

  // Returns the names of the features in sequential order (i. e. `names()[i].offset == i`)
  std::vector<Name> names() const;

  // Returns the number of features
  size_t count() const { return count_; }

  // Returns the list of bundles which comprise this feature set
  const std::vector<Bundle> &bundles() const { return bundles_; }

  // Creates an empty feature set
  Features() = default;

private:
  Features(std::vector<Bundle> bundles, const size_t count)
      : bundles_(std::move(bundles)), count_(count) {}

  template <typename ThisType, typename Callback>
  static void iterate(ThisType &obj, Callback callback);

  friend struct Private::BundleGroupImpl;

  std::vector<Bundle> bundles_;
  size_t count_;
};

}  // namespace SoFEval::Feat

#endif  // SOF_EVAL_FEAT_FEAT_INCLUDED
