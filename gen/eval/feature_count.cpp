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

  p.headerGuard("SOF_EVAL_FEATURE_COUNT_INCLUDED");
  p.skip();
  auto ns = p.inNamespace("SoFEval");
  p.skip();
  p.line() << "// Total number of features";
  p.line() << "constexpr size_t FEATURE_COUNT = " << features.count() << ";";
  p.skip();

  return 0;
}
