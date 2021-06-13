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

#ifndef SOF_BENCH_CORE_BENCH_BOARDS_INCLUDED
#define SOF_BENCH_CORE_BENCH_BOARDS_INCLUDED

// Helper macro to add a board on which the benchmarks can be run
#define SOF_DECLARE_BOARD(name, fen) constexpr const char *g_fen##name = fen

SOF_DECLARE_BOARD(Initial, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
SOF_DECLARE_BOARD(Sicilian, "r1b1k2r/2qnbppp/p2ppn2/1p4B1/3NPPP1/2N2Q2/PPP4P/2KR1B1R w kq - 0 11");
SOF_DECLARE_BOARD(Middle, "1rq1r1k1/1p3ppp/pB3n2/3ppP2/Pbb1P3/1PN2B2/2P2QPP/R1R4K w - - 1 21");
SOF_DECLARE_BOARD(OpenPosition, "4r1k1/3R1ppp/8/5P2/p7/6PP/4pK2/1rN1B3 w - - 4 43");
SOF_DECLARE_BOARD(Queens, "6K1/8/8/1k3q2/3Q4/8/8/8 w - - 0 1");
SOF_DECLARE_BOARD(PawnsMove, "4k3/pppppppp/8/8/8/8/PPPPPPPP/4K3 w - - 0 1");
SOF_DECLARE_BOARD(PawnsAttack, "4k3/8/8/pppppppp/PPPPPPPP/8/8/4K3 w - - 0 1");
SOF_DECLARE_BOARD(PawnsPromote, "8/PPPPPPPP/8/2k1K3/8/8/pppppppp/8 w - - 0 1");
SOF_DECLARE_BOARD(Cydonia, "5K2/1N1N1N2/8/1N1N1N2/1n1n1n2/8/1n1n1n2/5k2 w - - 0 1");
SOF_DECLARE_BOARD(Max, "3Q4/1Q4Q1/4Q3/2Q4R/Q4Q2/3Q4/NR4Q1/kN1BB1K1 w - - 0 1");

#undef SOF_DECLARE_BOARD

#endif  // SOF_BENCH_CORE_BENCH_BOARDS_INCLUDED
