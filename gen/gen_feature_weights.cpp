#include <jsoncpp/json/json.h>

#include <iostream>

#include "common.h"
#include "core/types.h"
#include "eval/feat/feat.h"
#include "util/misc.h"
#include "util/strutil.h"

using namespace SoFCore;
using namespace SoFEval::Feat;

std::string formatName(const Name &name) {
  std::string result;
  for (const char ch : name.name) {
    if (ch == '.') {
      result += '_';
      continue;
    }
    result += SoFUtil::asciiToUpper(ch);
  }
  return result;
}

struct Psq {
  std::vector<std::string> data[16];
  std::string kingsideCastling[2];
  std::string queensideCastling[2];
};

Psq moldPsq(const PsqBundle &bundle) {
  Psq result;
  for (size_t i = 0; i < 16; ++i) {
    result.data[i].assign(64, "Pair::from(empty())");
  }

  // Precalculate the piece-square table itself. Note that we must be very careful not to use `+`
  // and `-` operators, because this would dramatically increase the compilation time in combination
  // with `Coefs`
  for (Piece piece :
       {Piece::Pawn, Piece::King, Piece::Knight, Piece::Bishop, Piece::Rook, Piece::Queen}) {
    for (Color color : {Color::White, Color::Black}) {
      for (coord_t i = 0; i < 64; ++i) {
        auto num2 = [&](size_t first, size_t second) {
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

        const size_t pos = (color == Color::White) ? i : coordFlipX(i);
        result.data[makeCell(color, piece)][pos] = cost;
      }
    }
  }

  // Precalculate position cost changes after castling
  const std::string whiteKing = std::to_string(makeCell(Color::White, Piece::King));
  const std::string whiteRook = std::to_string(makeCell(Color::White, Piece::Rook));
  const std::string parentName = formatName(bundle.name());
  const std::string kingsideCastling =
      parentName + "[" + whiteKing + "][" + std::to_string(makeCoord(7, 6)) + "]" +          //
      " - " + parentName + "[" + whiteKing + "][" + std::to_string(makeCoord(7, 4)) + "]" +  //
      " + " + parentName + "[" + whiteRook + "][" + std::to_string(makeCoord(7, 5)) + "]" +  //
      " - " + parentName + "[" + whiteRook + "][" + std::to_string(makeCoord(7, 7)) + "]";
  const std::string queensideCastling =
      parentName + "[" + whiteKing + "][" + std::to_string(makeCoord(7, 2)) + "]" +          //
      " - " + parentName + "[" + whiteKing + "][" + std::to_string(makeCoord(7, 4)) + "]" +  //
      " + " + parentName + "[" + whiteRook + "][" + std::to_string(makeCoord(7, 3)) + "]" +  //
      " - " + parentName + "[" + whiteRook + "][" + std::to_string(makeCoord(7, 0)) + "]";
  result.kingsideCastling[static_cast<size_t>(Color::White)] = kingsideCastling;
  result.kingsideCastling[static_cast<size_t>(Color::Black)] = "-(" + kingsideCastling + ")";
  result.queensideCastling[static_cast<size_t>(Color::White)] = queensideCastling;
  result.queensideCastling[static_cast<size_t>(Color::Black)] = "-(" + queensideCastling + ")";

  return result;
}

void fillWeights(std::ostream &out, const Features &features, size_t indent) {
  for (const auto &bundle : features.bundles()) {
    if (auto b = bundle.asSingle()) {
      out << std::string(indent, ' ') << "static constexpr T " << formatName(b->name())
          << " = number(" << b->name().offset << ");" << std::endl;
      continue;
    }
    if (auto b = bundle.asArray()) {
      out << std::string(indent, ' ') << "static constexpr T " << formatName(b->name())
          << "[] = {\n";
      for (size_t idx = 0; idx < b->count(); ++idx) {
        out << std::string(indent + 2, ' ') << "number(" << b->name().offset + idx << ")";
        out << ((idx + 1 == b->count()) ? "\n" : ",\n");
      }
      out << std::string(indent, ' ') << "};\n";
      continue;
    }
    if (auto b = bundle.asPsq()) {
      auto psq = moldPsq(*b);
      out << std::string(indent, ' ') << "static constexpr Pair " << formatName(b->name())
          << "[16][64] = {\n";
      for (size_t i = 0; i < 16; ++i) {
        out << std::string(indent + 2, ' ') << "{\n";
        for (size_t j = 0; j < 64; ++j) {
          out << std::string(indent + 4, ' ') << psq.data[i][j];
          out << ((j + 1 == 64) ? "\n" : ",\n");
        }
        out << std::string(indent + 2, ' ') << "}";
        out << ((i + 1 == 16) ? "\n" : ",\n");
      }
      out << std::string(indent, ' ') << "};\n";

      auto outCastling = [&](const char *name, std::string value[2]) {
        out << std::string(indent, ' ') << "static constexpr Pair " << formatName(b->name()) << "_"
            << name << "_UPD[2] = {\n";
        out << std::string(indent + 2, ' ') << value[0] << ",\n";
        out << std::string(indent + 2, ' ') << value[1] << "\n";
        out << std::string(indent, ' ') << "};\n";
      };

      outCastling("KINGSIDE", psq.kingsideCastling);
      outCastling("QUEENSIDE", psq.queensideCastling);
      continue;
    }
    SoFUtil::panic("Unknown bundle type");
  }
}

int doGenerate(std::ostream &out, Json::Value json) {
  LoadResult<Features> maybeFeatures = Features::load(json);
  if (maybeFeatures.isErr()) {
    std::cerr << "Error extracting features: " << maybeFeatures.unwrapErr().description
              << std::endl;
    return 1;
  }
  Features features = maybeFeatures.unwrap();

  out << "#ifndef SOF_EVAL_PRIVATE_WEIGHTS_INCLUDED\n";
  out << "#define SOF_EVAL_PRIVATE_WEIGHTS_INCLUDED\n";
  out << "\n";
  out << "#include \"eval/score.h\"\n";
  out << "#include \"eval/coefs.h\"\n";
  out << "#include \"eval/types.h\"\n";
  out << "\n";
  out << "namespace SoFEval::Private {\n";
  out << "\n";
  out << "// Base struct that helps to declare weights for score type `T`\n";
  out << "template <typename T>\n";
  out << "struct WeightTraits {};\n";
  out << "\n";
  out << "template <>\n";
  out << "struct WeightTraits<score_t> {\n";
  out << "private:\n";
  out << "  static ";
  printIntArray(out, features.extract(), "WEIGHTS", "score_t", 2);
  out << "\n";
  out << "public:\n";
  out << "  static constexpr score_t empty() { return 0; }\n";
  out << "  static constexpr score_t number(size_t num) { return WEIGHTS[num]; }\n";
  out << "  static constexpr score_t number(size_t n1, size_t n2) {\n";
  out << "    return WEIGHTS[n1] + WEIGHTS[n2];\n";
  out << "  }\n";
  out << "\n";
  out << "  static constexpr score_t negNumber(size_t num) { return -WEIGHTS[num]; }\n";
  out << "  static constexpr score_t negNumber(size_t n1, size_t n2) {\n";
  out << "    return -(WEIGHTS[n1] + WEIGHTS[n2]);\n";
  out << "  }\n";
  out << "};\n";
  out << "\n";
  out << "template <>\n";
  out << "struct WeightTraits<Coefs> {\n";
  out << "  static constexpr Coefs empty() { return Coefs::zeroed(); }\n";
  out << "  static constexpr Coefs number(size_t num) { return makeCoefs(1, num); }\n";
  out << "  static constexpr Coefs number(size_t n1, size_t n2) {\n";
  out << "    return makeCoefs(1, n1, n2);\n";
  out << "  }\n";
  out << "\n";
  out << "  static constexpr Coefs negNumber(size_t num) { return makeCoefs(-1, num); }\n";
  out << "  static constexpr Coefs negNumber(size_t n1, size_t n2) {\n";
  out << "    return makeCoefs(-1, n1, n2);\n";
  out << "  }\n";
  out << "};\n";
  out << "\n";
  out << "// Keeps weights for score type `T`\n";
  out << "template <typename T>\n";
  out << "struct Weights : private WeightTraits<T> {\n";
  out << "private:\n";
  out << "  using WeightTraits<T>::empty;\n";
  out << "  using WeightTraits<T>::number;\n";
  out << "  using WeightTraits<T>::negNumber;\n";
  out << "\n";
  out << "public:\n";
  out << "  using Pair = typename ScoreTraits<T>::Pair;\n";
  fillWeights(out, features, 2);
  out << "};\n";
  out << "\n";
  out << "}  // namespace SoFEval::Private\n";
  out << "\n";
  out << "#endif  // SOF_EVAL_PRIVATE_WEIGHTS_INCLUDED\n";

  return 0;
}
