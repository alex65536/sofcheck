#include "common.h"
#include "eval/feat/feat.h"
#include "util/misc.h"

using namespace SoFEval::Feat;
using SoFUtil::panic;

int doGenerate(SourcePrinter &p, const Json::Value &json) {
  const Features features = Features::load(json).okOrErr(
      [](const auto err) { panic("Error extracting features: " + err.description); });

  p.headerGuard("SOF_EVAL_PRIVATE_WEIGHT_VALUES_INCLUDED");
  p.skip();
  p.include("eval/score.h");
  p.skip();
  auto ns = p.inNamespace("SoFEval::Private");
  p.skip();
  p.array("WEIGHT_VALUES", "score_t", features.extract());
  p.skip();

  return 0;
}
