#include <jsoncpp/json/json.h>

#include <iostream>

#include "eval/feat/feat.h"

using namespace SoFEval::Feat;

int doGenerate(std::ostream &out, const Json::Value &json) {
  LoadResult<Features> maybeFeatures = Features::load(json);
  if (maybeFeatures.isErr()) {
    std::cerr << "Error extracting features: " << maybeFeatures.unwrapErr().description
              << std::endl;
    return 1;
  }
  Features features = maybeFeatures.unwrap();

  out << "#ifndef SOF_EVAL_FEATURE_COUNT_INCLUDED\n";
  out << "#define SOF_EVAL_FEATURE_COUNT_INCLUDED\n";
  out << "\n";
  out << "namespace SoFEval {\n";
  out << "// Total number of features\n";
  out << "constexpr size_t FEATURE_COUNT = " << features.count() << ";\n";
  out << "}  // namespace SoFEval\n";
  out << "\n";
  out << "#endif  // SOF_EVAL_FEATURE_COUNT_INCLUDED\n";

  return 0;
}
