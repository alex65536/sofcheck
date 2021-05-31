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

  p.startHeaderGuard("SOF_EVAL_PRIVATE_WEIGHT_VALUES_INCLUDED");
  p.skip();
  p.line() << "#include \"eval/score.h\"";
  p.skip();
  p.line() << "namespace SoFEval::Private {";
  p.skip();
  p.array("WEIGHT_VALUES", "score_t", features.extract());
  p.skip();
  p.line() << "}  // namespace SoFEval::Private";
  p.skip();
  p.endHeaderGuard();

  return 0;
}
