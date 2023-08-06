// This file is part of SoFCheck
//
// Copyright (c) 2020, 2022-2023 Alexander Kernozhitsky and SoFCheck contributors
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

#ifndef SOF_CORE_PRIVATE_BITBOARD_INCLUDED
#define SOF_CORE_PRIVATE_BITBOARD_INCLUDED

#include "core/types.h"

namespace SoFCore::Private {

constexpr bitboard_t BB_ROW_FRAME = 0x8181818181818181;
constexpr bitboard_t BB_COL_FRAME = 0xff000000000000ff;
constexpr bitboard_t BB_DIAG_FRAME = 0xff818181818181ff;

constexpr bitboard_t BB_CASTLING_KINGSIDE_PASS = 0x60;
constexpr bitboard_t BB_CASTLING_QUEENSIDE_PASS = 0x0e;

constexpr bitboard_t BB_CASTLING_BLACK_KINGSIDE_SRCS = 0x90;
constexpr bitboard_t BB_CASTLING_BLACK_QUEENSIDE_SRCS = 0x11;
constexpr bitboard_t BB_CASTLING_BLACK_ALL_SRCS = 0x91;

constexpr bitboard_t BB_CASTLING_WHITE_KINGSIDE_SRCS = BB_CASTLING_BLACK_KINGSIDE_SRCS << 56;
constexpr bitboard_t BB_CASTLING_WHITE_QUEENSIDE_SRCS = BB_CASTLING_BLACK_QUEENSIDE_SRCS << 56;
constexpr bitboard_t BB_CASTLING_WHITE_ALL_SRCS = BB_CASTLING_BLACK_ALL_SRCS << 56;

constexpr bitboard_t BB_CASTLING_ALL_SRCS = BB_CASTLING_WHITE_ALL_SRCS | BB_CASTLING_BLACK_ALL_SRCS;

static_assert(BB_CASTLING_BLACK_ALL_SRCS ==
              (BB_CASTLING_BLACK_KINGSIDE_SRCS | BB_CASTLING_BLACK_QUEENSIDE_SRCS));

}  // namespace SoFCore::Private

#endif  // SOF_CORE_PRIVATE_BITBOARD_INCLUDED
