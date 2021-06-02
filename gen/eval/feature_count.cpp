#include "common.h"
#include "eval/feat/feat.h"
#include "util/misc.h"

using namespace SoFEval::Feat;
using SoFUtil::panic;

int doGenerate(SourcePrinter &p, const Json::Value &json) {
  const Features features = Features::load(json).okOrErr(
      [](const auto err) { panic("Error extracting features: " + err.description); });

  p.headerGuard("SOF_EVAL_FEATURE_COUNT_INCLUDED");
  p.skip();
  auto ns = p.inNamespace("SoFEval");
  p.skip();
  p.line() << "// Total number of features";
  p.line() << "constexpr size_t FEATURE_COUNT = " << features.count() << ";";
  p.skip();

  return 0;
}
