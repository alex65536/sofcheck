#include <iomanip>
#include <iostream>
#include <vector>

#include "common.h"
#include "core/types.h"
#include "search/private/score.h"

using namespace SoFCore;
using namespace SoFSearch::Private;

static constexpr score_t PIECE_COSTS[6] = {100, 0, 350, 350, 525, 1000};

static constexpr score_t PIECE_CELL_BONUS_TABLE[6][64] = {
    {
        // Pawn
        0,  0,  0,  0,   0,   0,  0,  0,   //
        50, 50, 50, 50,  50,  50, 50, 50,  //
        30, 30, 30, 30,  30,  30, 30, 30,  //
        15, 15, 20, 25,  25,  20, 15, 15,  //
        5,  5,  10, 20,  20,  10, 5,  5,   //
        0,  0,  0,  0,   0,   0,  0,  0,   //
        5,  5,  5,  -20, -20, 5,  5,  5,   //
        0,  0,  0,  0,   0,   0,  0,  0,   //
    },
    {
        // King
        -50, -50, -50, -50, -50, -50, -50, -50,  //
        -50, -50, -50, -50, -50, -50, -50, -50,  //
        -50, -50, -50, -50, -50, -50, -50, -50,  //
        -50, -50, -50, -50, -50, -50, -50, -50,  //
        -30, -40, -50, -50, -50, -50, -40, -30,  //
        -10, -20, -30, -30, -30, -30, -20, -10,  //
        20,  20,  -5,  -10, -10, -5,  20,  20,   //
        10,  30,  0,   -10, -10, 0,   30,  10,   //
    },
    {
        // Knight
        -50, -30, -20, -10, -10, -20, -30, -50,  //
        -30, -10, 0,   5,   5,   0,   -10, -30,  //
        -20, 0,   5,   10,  10,  5,   0,   -20,  //
        -10, 5,   10,  20,  20,  10,  0,   -10,  //
        -10, 5,   10,  20,  20,  10,  0,   -10,  //
        -20, 0,   5,   10,  10,  5,   0,   -20,  //
        -30, -10, 0,   5,   5,   0,   -10, -30,  //
        -50, -30, -20, -10, -10, -20, -30, -50,  //
    },
    {
        // Bishop
        0,   -10, -10, -10, -10, -10, -10, 0,    //
        -10, 15,  5,   5,   5,   5,   15,  -10,  //
        -10, 5,   15,  10,  10,  15,  5,   -10,  //
        -10, 5,   10,  15,  15,  10,  5,   -10,  //
        -10, 5,   10,  15,  15,  10,  5,   -10,  //
        -10, 5,   15,  10,  10,  15,  5,   -10,  //
        -10, 15,  5,   5,   5,   5,   15,  -10,  //
        0,   -10, -10, -10, -10, -10, -10, 0,    //
    },
    {
        // Rook
        25,  25, 25, 25, 25, 25, 25, 25,   //
        50,  50, 50, 50, 50, 50, 50, 50,   //
        -5,  0,  5,  10, 10, 5,  0,  -5,   //
        -5,  0,  5,  10, 10, 5,  0,  -5,   //
        -5,  0,  5,  10, 10, 5,  0,  -5,   //
        -5,  0,  5,  10, 10, 5,  0,  -5,   //
        -5,  0,  5,  10, 10, 5,  0,  -5,   //
        -10, -5, 5,  10, 10, 5,  -5, -10,  //
    },
    {
        // Queen
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
    }};

static constexpr score_t PIECE_CELL_END_BONUS_TABLE[6][64] = {
    {
        // Pawn
        0,  0,  0,   0,   0,   0,   0,  0,   //
        50, 75, 100, 100, 100, 100, 75, 50,  //
        30, 40, 50,  50,  50,  50,  40, 30,  //
        20, 20, 25,  25,  25,  25,  20, 20,  //
        10, 10, 10,  10,  10,  10,  10, 10,  //
        0,  0,  0,   0,   0,   0,   0,  0,   //
        0,  0,  0,   0,   0,   0,   0,  0,   //
        0,  0,  0,   0,   0,   0,   0,  0,   //
    },
    {
        // King

        -50, -30, -20, -20, -20, -20, -30, -50,  //
        -30, 0,   10,  20,  20,  10,  0,   -30,  //
        -20, 5,   20,  40,  40,  20,  5,   -20,  //
        -20, 10,  40,  50,  50,  40,  10,  -20,  //
        -20, 10,  40,  50,  50,  40,  10,  -20,  //
        -20, 5,   20,  40,  40,  20,  5,   -20,  //
        -30, 0,   10,  20,  20,  10,  0,   -30,  //
        -50, -30, -20, -20, -20, -20, -30, -50,  //
    },
    {
        // Knight
        -50, -30, -20, -10, -10, -20, -30, -50,  //
        -30, -10, 0,   5,   5,   0,   -10, -30,  //
        -20, 0,   5,   10,  10,  5,   0,   -20,  //
        -10, 5,   10,  20,  20,  10,  0,   -10,  //
        -10, 5,   10,  20,  20,  10,  0,   -10,  //
        -20, 0,   5,   10,  10,  5,   0,   -20,  //
        -30, -10, 0,   5,   5,   0,   -10, -30,  //
        -50, -30, -20, -10, -10, -20, -30, -50,  //
    },
    {
        // Bishop
        0,   -10, -10, -10, -10, -10, -10, 0,    //
        -10, 15,  5,   5,   5,   5,   15,  -10,  //
        -10, 5,   15,  10,  10,  15,  5,   -10,  //
        -10, 5,   10,  15,  15,  10,  5,   -10,  //
        -10, 5,   10,  15,  15,  10,  5,   -10,  //
        -10, 5,   15,  10,  10,  15,  5,   -10,  //
        -10, 15,  5,   5,   5,   5,   15,  -10,  //
        0,   -10, -10, -10, -10, -10, -10, 0,    //
    },
    {
        // Rook
        -20, -10, -10, -10, -10, -10, -10, -20,  //
        -10, 0,   0,   0,   0,   0,   0,   -10,  //
        -10, 0,   0,   0,   0,   0,   0,   -10,  //
        -10, 0,   0,   0,   0,   0,   0,   -10,  //
        -10, 0,   0,   0,   0,   0,   0,   -10,  //
        -10, 0,   0,   0,   0,   0,   0,   -10,  //
        -10, 0,   0,   0,   0,   0,   0,   -10,  //
        -20, -10, -10, -10, -10, -10, -10, -20,  //
    },
    {
        // Queen
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
        0, 0, 0, 0, 0, 0, 0, 0,  //
    }};

void printSinglePieceTable(std::ostream &out, const std::vector<score_pair_t> &tab) {
  out << "  {\n";
  for (subcoord_t x = 0; x < 8; ++x) {
    out << "    ";
    for (subcoord_t y = 0; y < 8; ++y) {
      if (y != 0) {
        out << ", ";
      }
      out << tab[makeCoord(x, y)];
    }
    if (x != 7) {
      out << ",";
    }
    out << "\n";
  }
  out << "  }";
}

void printPieceSquareTables(std::ostream &out) {
  std::vector<score_pair_t> scores[16];
  for (auto &scoreVec : scores) {
    scoreVec.assign(64, makeScorePair(0));
  }

  // Generate piece-square tables
  for (Piece piece :
       {Piece::Pawn, Piece::King, Piece::Knight, Piece::Bishop, Piece::Rook, Piece::Queen}) {
    for (coord_t i = 0; i < 64; ++i) {
      const auto pieceIdx = static_cast<size_t>(piece);
      const score_t score = PIECE_COSTS[pieceIdx] + PIECE_CELL_BONUS_TABLE[pieceIdx][i];
      const score_t endScore = PIECE_COSTS[pieceIdx] + PIECE_CELL_END_BONUS_TABLE[pieceIdx][i];
      const score_pair_t scorePair = makeScorePair(score, endScore);
      scores[makeCell(Color::White, piece)][i] = scorePair;
      scores[makeCell(Color::Black, piece)][coordFlipX(i)] = -scorePair;
    }
  }
  out << "constexpr score_pair_t PIECE_SQUARE_TABLE[16][64] = {";
  for (size_t i = 0; i < 16; ++i) {
    printSinglePieceTable(out, scores[i]);
    if (i != 15) {
      out << ",\n";
    }
  }
  out << "\n};\n\n";

  // Precalculate position cost changes after castling
  constexpr cell_t whiteKing = makeCell(Color::White, Piece::King);
  constexpr cell_t whiteRook = makeCell(Color::White, Piece::Rook);
  const score_pair_t castlingKingsideUpdate =
      scores[whiteKing][makeCoord(7, 6)] - scores[whiteKing][makeCoord(7, 4)] +
      scores[whiteRook][makeCoord(7, 5)] - scores[whiteRook][makeCoord(7, 7)];
  const score_pair_t castlingQueensideUpdate =
      scores[whiteKing][makeCoord(7, 2)] - scores[whiteKing][makeCoord(7, 4)] +
      scores[whiteRook][makeCoord(7, 3)] - scores[whiteRook][makeCoord(7, 0)];
  out << "constexpr score_pair_t SCORE_CASTLING_KINGSIDE_UPD[2] = {" << castlingKingsideUpdate
      << ", " << -castlingKingsideUpdate << "};\n";
  out << "constexpr score_pair_t SCORE_CASTLING_QUEENSIDE_UPD[2] = {" << castlingQueensideUpdate
      << ", " << -castlingQueensideUpdate << "};\n";
}

void doGenerate(std::ostream &out) {
  out << "#ifndef SOF_SEARCH_PRIVATE_PIECE_SQUARE_TABLE_INCLUDED\n";
  out << "#define SOF_SEARCH_PRIVATE_PIECE_SQUARE_TABLE_INCLUDED\n";
  out << "\n";
  out << "#include \"core/types.h\"\n";
  out << "#include \"search/private/score.h\"\n";
  out << "\n";
  out << "namespace SoFSearch::Private {\n";
  out << "\n";

  printPieceSquareTables(out);
  out << "\n";

  out << "}  // namespace SoFSearch::Private\n";
  out << "\n";

  out << "#endif  // SOF_SEARCH_PRIVATE_PIECE_SQUARE_TABLE_INCLUDED\n";
}
