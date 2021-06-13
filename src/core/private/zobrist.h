// This file is part of SoFCheck
//
// Copyright (c) 2020 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_CORE_PRIVATE_ZOBRIST_INCLUDED
#define SOF_CORE_PRIVATE_ZOBRIST_INCLUDED

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
