#ifndef SOF_EVAL_FEAT_FEAT_INCLUDED
#define SOF_EVAL_FEAT_FEAT_INCLUDED

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "util/result.h"

namespace Json {
class Value;
}  // namespace Json

namespace SoFEval::Feat {

using weight_t = int32_t;

struct Name {
  size_t offset;
  std::string name;
};

struct LoadError {
  std::string description;
};

template <typename T>
using LoadResult = SoFUtil::Result<T, LoadError>;

using WeightVec = std::vector<weight_t>;

class SingleBundle {
public:
  static LoadResult<SingleBundle> load(const Name &name, const Json::Value &json);
  void save(Json::Value &json) const;
  void apply(const WeightVec &weights) { value_ = weights[name_.offset]; }
  void extract(WeightVec &weights) const { weights[name_.offset] = value_; }
  std::vector<Name> names() const { return {name_}; }
  size_t count() const { return 1; }
  const Name &name() const { return name_; }

  SingleBundle() = default;

private:
  SingleBundle(Name name, weight_t value) : name_(std::move(name)), value_(value) {}

  Name name_;
  weight_t value_;
};

class ArrayBundle {
public:
  static LoadResult<ArrayBundle> load(const Name &name, const Json::Value &json);
  void save(Json::Value &json) const;
  void apply(const WeightVec &weights);
  void extract(WeightVec &weights) const;
  std::vector<Name> names() const;
  size_t count() const { return values_.size(); }
  const Name &name() const { return name_; }

  ArrayBundle() = default;

private:
  ArrayBundle(Name name, std::vector<weight_t> values)
      : name_(std::move(name)), values_(std::move(values)) {}

  Name name_;
  std::vector<weight_t> values_;
};

class PsqBundle {
public:
  static LoadResult<PsqBundle> load(const Name &name, const Json::Value &json);
  void save(Json::Value &json) const;
  void apply(const WeightVec &weights);
  void extract(WeightVec &weights) const;
  std::vector<Name> names() const;
  size_t count() const;
  const Name &name() const { return name_; }

  const ArrayBundle &pieceCosts() const { return pieceCosts_; }
  const ArrayBundle &table(size_t idx) const { return tables_[idx]; }
  const ArrayBundle &endKingTable() const { return endKingTable_; }

  PsqBundle() = default;

private:
  static constexpr size_t PIECE_COUNT = 6;
  static constexpr const char *PIECE_NAMES[6] = {"pawn",   "king", "knight",
                                                 "bishop", "rook", "queen"};

  explicit PsqBundle(Name name) : name_(std::move(name)) {}

  Name name_;
  ArrayBundle pieceCosts_;
  ArrayBundle tables_[PIECE_COUNT];
  ArrayBundle endKingTable_;
};

class Bundle {
public:
  static LoadResult<Bundle> load(const Name &name, const Json::Value &json);

  void save(Json::Value &json) const {
    std::visit([&](const auto &x) { x.save(json); }, inner_);
  }

  void apply(const WeightVec &weights) {
    std::visit([&](auto &x) { x.apply(weights); }, inner_);
  }

  void extract(WeightVec &weights) const {
    std::visit([&](const auto &x) { x.extract(weights); }, inner_);
  }

  std::vector<Name> names() const {
    return std::visit([&](const auto &x) { return x.names(); }, inner_);
  }

  size_t count() const {
    return std::visit([&](const auto &x) { return x.count(); }, inner_);
  }

  const Name &name() const {
    return std::visit([&](const auto &x) -> const Name & { return x.name(); }, inner_);
  }

#define D_AS_BUNDLE(type)                                                 \
  type##Bundle *as##type() { return std::get_if<type##Bundle>(&inner_); } \
  const type##Bundle *as##type() const { return std::get_if<type##Bundle>(&inner_); }

  D_AS_BUNDLE(Single)
  D_AS_BUNDLE(Array)
  D_AS_BUNDLE(Psq)

#undef D_AS_BUNDLE

  Bundle() = default;

private:
  struct VariantItemTag {};

  template <typename T>
  Bundle(VariantItemTag, T value) : inner_(std::move(value)) {}

  std::variant<SingleBundle, ArrayBundle, PsqBundle> inner_;
};

class Features {
public:
  static LoadResult<Features> load(const Json::Value &json);
  void save(Json::Value &json) const;
  void apply(const WeightVec &weights);
  WeightVec extract() const;
  std::vector<Name> names() const;
  size_t count() const { return count_; }

  const std::vector<Bundle> bundles() const { return bundles_; }

  Features() = default;

private:
  Features(std::vector<Bundle> bundles, size_t count)
      : bundles_(std::move(bundles)), count_(count) {}

  std::vector<Bundle> bundles_;
  size_t count_;
};

}  // namespace SoFEval::Feat

#endif  // SOF_EVAL_FEAT_FEAT_INCLUDED
