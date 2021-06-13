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

#ifndef SOF_CORE_PRIVATE_BIT_CONSTS_INCLUDED
#define SOF_CORE_PRIVATE_BIT_CONSTS_INCLUDED

namespace SoFCore::Private {

constexpr bitboard_t BB_DIAG1[15] = {
    0x0000000000000001, 0x0000000000000102, 0x0000000000010204, 0x0000000001020408,
    0x0000000102040810, 0x0000010204081020, 0x0001020408102040, 0x0102040810204080,
    0x0204081020408000, 0x0408102040800000, 0x0810204080000000, 0x1020408000000000,
    0x2040800000000000, 0x4080000000000000, 0x8000000000000000,
};

constexpr bitboard_t BB_DIAG2[15] = {
    0x0100000000000000, 0x0201000000000000, 0x0402010000000000, 0x0804020100000000,
    0x1008040201000000, 0x2010080402010000, 0x4020100804020100, 0x8040201008040201,
    0x0080402010080402, 0x0000804020100804, 0x0000008040201008, 0x0000000080402010,
    0x0000000000804020, 0x0000000000008040, 0x0000000000000080,
};

constexpr bitboard_t BB_ROW[8] = {
    0x00000000000000ff, 0x000000000000ff00, 0x0000000000ff0000, 0x00000000ff000000,
    0x000000ff00000000, 0x0000ff0000000000, 0x00ff000000000000, 0xff00000000000000,
};

constexpr bitboard_t BB_COL[8] = {
    0x0101010101010101, 0x0202020202020202, 0x0404040404040404, 0x0808080808080808,
    0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080,
};

constexpr bitboard_t BB_CELLS_WHITE = 0xaa55aa55aa55aa55;
constexpr bitboard_t BB_CELLS_BLACK = 0x55aa55aa55aa55aa;

constexpr bitboard_t BB_ROW_FRAME = 0x8181818181818181;
constexpr bitboard_t BB_COL_FRAME = 0xff000000000000ff;
constexpr bitboard_t BB_DIAG_FRAME = 0xff818181818181ff;

constexpr bitboard_t BB_CASTLING_KINGSIDE_PASS = 0x60;
constexpr bitboard_t BB_CASTLING_QUEENSIDE_PASS = 0x0e;

constexpr bitboard_t BB_CASTLING_BLACK_KINGSIDE_SRCS = 0x90;
constexpr bitboard_t BB_CASTLING_BLACK_QUEENSIDE_SRCS = 0x11;
constexpr bitboard_t BB_CASTLING_WHITE_KINGSIDE_SRCS = BB_CASTLING_BLACK_KINGSIDE_SRCS << 56;
constexpr bitboard_t BB_CASTLING_WHITE_QUEENSIDE_SRCS = BB_CASTLING_BLACK_QUEENSIDE_SRCS << 56;

}  // namespace SoFCore::Private

#endif  // SOF_CORE_PRIVATE_BIT_CONSTS_INCLUDED
