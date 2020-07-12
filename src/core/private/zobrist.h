#ifndef SOF_CORE_PRIVATE_ZOBRIST_INCLUDED
#define SOF_CORE_PRIVATE_ZOBRIST_INCLUDED

#include <cstdint>

#include "core/types.h"

namespace SoFCore::Private {

extern board_hash_t g_zobristPieces[16][64];
extern board_hash_t g_zobristMoveSide;
extern board_hash_t g_zobristCastling[16];
extern board_hash_t g_zobristEnpassant[64];
extern board_hash_t g_zobristPieceCastlingKingside[2];
extern board_hash_t g_zobristPieceCastlingQueenside[2];

void initZobrist();

}  // namespace SoFCore::Private

#endif  // SOF_CORE_PRIVATE_ZOBRIST_INCLUDED
