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

#include "common.h"
#include "eval/feat/feat.h"
#include "util/misc.h"

using namespace SoFEval::Feat;
using SoFUtil::panic;

GeneratorInfo getGeneratorInfo() {
  return GeneratorInfo{"Parse weights of evaluation features from JSON"};
}

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
