#include "eval/feat/feat.h"

#include <json/json.h>

#include <algorithm>
#include <utility>

namespace SoFEval::Feat {

using SoFUtil::Err;  // NOLINT: clang-tidy thinks it's unused by some reason
using SoFUtil::Ok;   // NOLINT: clang-tidy thinks it's unused by some reason

LoadResult<SingleBundle> SingleBundle::load(const Name &name, const Json::Value &json) {
  if (!json.isInt()) {
    return Err(LoadError{name.name + " must be int"});
  }
  return Ok(SingleBundle(name, json.asInt()));
}

void SingleBundle::save(Json::Value &json) const { json = value_; }

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
  json.resize(values_.size());
  for (Json::ArrayIndex idx = 0; idx < json.size(); ++idx) {
    json[idx] = values_[idx];
  }
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
    const Name curName{name.offset + curOffset, name.name + "." + subName};
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
  pieceCosts_.save(json["cost"]);
  for (size_t idx = 0; idx < PIECE_COUNT; ++idx) {
    tables_[idx].save(json[PIECE_NAMES[idx]]);
  }
  endKingTable_.save(json["king_end"]);
}

void PsqBundle::apply(const WeightVec &weights) {
  pieceCosts_.apply(weights);
  for (ArrayBundle &item : tables_) {
    item.apply(weights);
  }
  endKingTable_.apply(weights);
}

void PsqBundle::extract(WeightVec &weights) const {
  pieceCosts_.extract(weights);
  for (const ArrayBundle &item : tables_) {
    item.extract(weights);
  }
  endKingTable_.extract(weights);
}

std::vector<Name> PsqBundle::names() const {
  std::vector<Name> result;
  result.reserve(count());

  auto append = [&](const std::vector<Name> &names) {
    result.insert(result.end(), names.begin(), names.end());
  };

  append(pieceCosts_.names());
  for (const ArrayBundle &item : tables_) {
    append(item.names());
  }
  append(endKingTable_.names());

  return result;
}

size_t PsqBundle::count() const {
  size_t result = 0;

  result += pieceCosts_.count();
  for (const ArrayBundle &item : tables_) {
    result += item.count();
  }
  result += endKingTable_.count();

  return result;
}

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

LoadResult<Features> Features::load(const Json::Value &json) {
  if (!json.isObject()) {
    return Err(LoadError{"Feature JSON must be object"});
  }

  std::vector<std::string> members = json.getMemberNames();
  std::sort(members.begin(), members.end());

  std::vector<Bundle> bundles(members.size());
  size_t counter = 0;
  for (size_t idx = 0; idx < members.size(); ++idx) {
    const std::string &member = members[idx];
    SOF_TRY_ASSIGN(bundles[idx], Bundle::load(Name{counter, member}, json[member]));
    counter += bundles[idx].count();
  }

  return Ok(Features(std::move(bundles), counter));
}

void Features::save(Json::Value &json) const {
  json = Json::Value(Json::objectValue);
  for (const Bundle &bundle : bundles_) {
    bundle.save(json[bundle.name().name]);
  }
}

void Features::apply(const WeightVec &weights) {
  SOF_ASSERT(weights.size() == count());
  for (Bundle &bundle : bundles_) {
    bundle.apply(weights);
  }
}

WeightVec Features::extract() const {
  WeightVec result(count());
  for (const Bundle &bundle : bundles_) {
    bundle.extract(result);
  }
  return result;
}

std::vector<Name> Features::names() const {
  std::vector<Name> result;
  result.reserve(count());

  for (const Bundle &bundle : bundles_) {
    std::vector<Name> cur = bundle.names();
    result.insert(result.end(), cur.begin(), cur.end());
  }

  // Ensure that the features arrive in the sequential order
  SOF_ASSERT(result.size() == count());
  for (size_t idx = 0; idx < result.size(); ++idx) {
    SOF_ASSERT(result[idx].offset == idx);
  }

  return result;
}

}  // namespace SoFEval::Feat
