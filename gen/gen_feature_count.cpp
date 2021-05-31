#include <jsoncpp/json/json.h>

#include "common.h"
#include "eval/feat/feat.h"

using namespace SoFEval::Feat;

int doGenerate(SourcePrinter &p, const Json::Value &json) {
  LoadResult<Features> maybeFeatures = Features::load(json);
  if (maybeFeatures.isErr()) {
    std::cerr << "Error extracting features: " << maybeFeatures.unwrapErr().description
              << std::endl;
    return 1;
  }
  const Features features = maybeFeatures.unwrap();

  p.startHeaderGuard("SOF_EVAL_FEATURE_COUNT_INCLUDED");
  p.skip();
  p.line() << "namespace SoFEval {";
  p.line() << "// Total number of features";
  p.line() << "constexpr size_t FEATURE_COUNT = " << features.count() << ";";
  p.line() << "}  // namespace SoFEval";
  p.skip();
  p.endHeaderGuard();

  return 0;
}
