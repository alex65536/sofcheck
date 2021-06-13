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

#include <iomanip>
#include <iostream>

#include "common.h"
#include "core/types.h"
#include "eval/feat/feat.h"
#include "util/misc.h"
#include "util/strutil.h"

using namespace SoFCore;
using namespace SoFEval::Feat;
using SoFUtil::panic;

std::string formatName(const Name &name) {
  std::string result = name.name;
  SoFUtil::asciiToUpper(result);
  SoFUtil::replace(result, '.', '_');
  return result;
}

struct Psq {
  std::vector<std::string> data[16];
  std::string kingsideCastling[2];
  std::string queensideCastling[2];
};

Psq psqFromBundle(const PsqBundle &bundle) {
  Psq result;
  for (auto &vec : result.data) {
    vec.assign(64, "Pair::from(empty())");
  }

  // Precalculate the piece-square table itself
  for (Piece piece :
       {Piece::Pawn, Piece::King, Piece::Knight, Piece::Bishop, Piece::Rook, Piece::Queen}) {
    for (Color color : {Color::White, Color::Black}) {
      for (coord_t i = 0; i < 64; ++i) {
        auto num2 = [&color](size_t first, size_t second) {
          const std::string name = (color == Color::White) ? "number" : "negNumber";
          return name + "(" + std::to_string(first) + ", " + std::to_string(second) + ")";
        };

        const auto pieceIdx = static_cast<size_t>(piece);
        const size_t pieceFeat = bundle.pieceCosts().name().offset + pieceIdx;
        const size_t cellFeat = bundle.table(pieceIdx).name().offset + i;
        std::string cost;
        if (piece == Piece::King) {
          const size_t kingFeat = bundle.endKingTable().name().offset + i;
          cost = "Pair::from(" + num2(pieceFeat, cellFeat) + ", " + num2(pieceFeat, kingFeat) + ")";
        } else {
          cost = "Pair::from(" + num2(pieceFeat, cellFeat) + ")";
        }

        const coord_t pos = (color == Color::White) ? i : coordFlipX(i);
        result.data[makeCell(color, piece)][pos] = cost;
      }
    }
  }

  auto large = [](const std::string &str) { return "LargePair(" + str + ")"; };

  // Precalculate position cost changes after castling
  const std::string whiteKing = std::to_string(makeCell(Color::White, Piece::King));
  const std::string whiteRook = std::to_string(makeCell(Color::White, Piece::Rook));
  const std::string parentName = formatName(bundle.name());
  const std::string kingsideCastling =
      large(parentName + "[" + whiteKing + "][" + std::to_string(makeCoord(7, 6)) + "]") + " - " +
      large(parentName + "[" + whiteKing + "][" + std::to_string(makeCoord(7, 4)) + "]") + " + " +
      large(parentName + "[" + whiteRook + "][" + std::to_string(makeCoord(7, 5)) + "]") + " - " +
      large(parentName + "[" + whiteRook + "][" + std::to_string(makeCoord(7, 7)) + "]");
  const std::string queensideCastling =
      large(parentName + "[" + whiteKing + "][" + std::to_string(makeCoord(7, 2)) + "]") + " - " +
      large(parentName + "[" + whiteKing + "][" + std::to_string(makeCoord(7, 4)) + "]") + " + " +
      large(parentName + "[" + whiteRook + "][" + std::to_string(makeCoord(7, 3)) + "]") + " - " +
      large(parentName + "[" + whiteRook + "][" + std::to_string(makeCoord(7, 0)) + "]");
  result.kingsideCastling[static_cast<size_t>(Color::White)] = kingsideCastling;
  result.kingsideCastling[static_cast<size_t>(Color::Black)] = "-(" + kingsideCastling + ")";
  result.queensideCastling[static_cast<size_t>(Color::White)] = queensideCastling;
  result.queensideCastling[static_cast<size_t>(Color::Black)] = "-(" + queensideCastling + ")";

  return result;
}

void fillWeights(SourcePrinter &p, const Features &features) {
  for (const auto &bundle : features.bundles()) {
    if (const auto *b = bundle.asSingle()) {
      p.line() << "static constexpr Item " << formatName(b->name()) << " = number("
               << b->name().offset << ");";
      continue;
    }

    if (const auto *b = bundle.asArray()) {
      p.lineStart() << "static constexpr Item " << formatName(b->name()) << "[" << b->count()
                    << "] = ";
      p.arrayBody(b->count(), [&](const size_t idx) {
        p.stream() << "number(" << b->name().offset + idx << ")";
      });
      p.stream() << ";\n";
      continue;
    }

    if (const auto *b = bundle.asPsq()) {
      const auto psq = psqFromBundle(*b);
      p.lineStart() << "static constexpr Pair " << formatName(b->name()) << "[16][64] = ";
      p.arrayBody(16, [&](const size_t i) {
        p.arrayBody(64, [&](const size_t j) { p.stream() << psq.data[i][j]; });
      });
      p.stream() << ";\n";

      auto outCastling = [&](const char *name, const std::string value[2]) {
        p.line() << "static constexpr LargePair " << formatName(b->name()) << "_" << name
                 << "_UPD[2] = {";
        static_assert(static_cast<int>(Color::White) == 0 && static_cast<int>(Color::Black) == 1);
        p.indent(2);
        p.line() << "/* White */ " << value[0] << ",";
        p.line() << "/* Black */ " << value[1];
        p.outdent(2);
        p.line() << "};";
      };

      outCastling("KINGSIDE", psq.kingsideCastling);
      outCastling("QUEENSIDE", psq.queensideCastling);
      continue;
    }

    SoFUtil::panic("Unknown bundle type");
  }
}

int doGenerate(SourcePrinter &p, const Json::Value &json) {
  const Features features = Features::load(json).okOrErr(
      [](const auto err) { panic("Error extracting features: " + err.description); });

  p.headerGuard("SOF_EVAL_PRIVATE_WEIGHTS_INCLUDED");
  p.skip();
  p.include("eval/private/weight_traits.h");
  p.skip();
  auto ns = p.inNamespace("SoFEval::Private");
  p.skip();
  p.line() << "// Keeps weights for score type `T`";
  p.line() << "template <typename T>";
  p.line() << "struct Weights : private WeightTraits<T> {";
  p.line() << "private:";
  p.line() << "  using WeightTraits<T>::empty;";
  p.line() << "  using WeightTraits<T>::number;";
  p.line() << "  using WeightTraits<T>::negNumber;";
  p.skip();
  p.line() << "public:";
  p.line() << "  using typename WeightTraits<T>::Item;";
  p.line() << "  using typename WeightTraits<T>::LargeItem;";
  p.line() << "  using Pair = typename ScoreTraits<Item>::Pair;";
  p.line() << "  using LargePair = typename ScoreTraits<LargeItem>::Pair;";
  p.skip();
  p.line() << "  // Here comes the weights declaration";
  p.indent(1);
  fillWeights(p, features);
  p.outdent(1);
  p.line() << "};";
  p.skip();

  return 0;
}
