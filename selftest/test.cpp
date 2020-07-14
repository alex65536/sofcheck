#include "chess_intf.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "util.h"

// Print all the move chains while depthDump?
// #define DEPTH_DUMP_TRACE_CHAINS

// Run depthDump on big recursion depths (consumes more time for test)?
#define DEPTH_DUMP_LARGE

// Run self tests?
#define RUN_SELF_TESTS

#if defined(RUN_SELF_TESTS) && !defined(CHESS_INTF_HAS_SELF_TESTS)
#undef RUN_SELF_TESTS
#endif

void depthDump(ChessIntf::Board &board, uint64_t &hsh, int d, bool checkHeatmaps, std::string &moveChain) {
    using namespace ChessIntf;

    if (d == 0) {
#ifdef DEPTH_DUMP_TRACE_CHAINS
        std::cout << "cur-chain: " << moveChain << "\n";
#endif
        if (checkHeatmaps) {
            for (bool color : {true, false}) {
                for (char y = '8'; y >= '1'; --y) {
                    uint64_t data = 0;
                    for (char x = 'a'; x <= 'h'; ++x) {
                        data *= 2;
                        data += isAttacked(board, color, x, y);
                    }
                    hsh *= 2579;
                    hsh += data;
                }
            }
        }
        return;
    }
    MoveList moves;
    generateMoves(board, moves);
    int cnt = getMoveCount(moves);
    std::pair<int, int> moveOrd[240];
    for (int i = 0; i < cnt; ++i) {
        moveOrd[i].first = getMoveHash(board, getMove(moves, i));
        moveOrd[i].second = i;
    }
    std::sort(moveOrd, moveOrd + cnt);
    for (int i = 0; i < cnt; ++i) {
        hsh *= 2579;
        hsh += static_cast<uint64_t>(moveOrd[i].first);
    }
    for (int i = 0; i < cnt; ++i) {
        const Move &move = getMove(moves, moveOrd[i].second);
        MovePersistence persistence;
#ifdef DEPTH_DUMP_TRACE_CHAINS
        char str[6];
        moveStr(board, move, str);
        size_t wasLen = moveChain.size();
#endif
        makeMove(board, move, persistence);
        if (!isOpponentKingAttacked(board)) {
#ifdef DEPTH_DUMP_TRACE_CHAINS
            moveChain += str;
            moveChain += " ";
#endif
            depthDump(board, hsh, d-1, checkHeatmaps, moveChain);
#ifdef DEPTH_DUMP_TRACE_CHAINS
            moveChain.resize(wasLen);
#endif
        }
        unmakeMove(board, move, persistence);
    }
}

inline std::vector<std::string> getMoveStrList(
    const ChessIntf::Board& board,
    const ChessIntf::MoveList &moves
) {
    using namespace ChessIntf;

    std::vector<std::string> moveList;
    moveList.reserve(getMoveCount(moves));
    for (int i = 0; i < getMoveCount(moves); ++i) {
        char str[6];
        moveStr(board, getMove(moves, i), str);
        moveList.emplace_back(str);
    }
    std::sort(moveList.begin(), moveList.end());
    return moveList;
}

std::string hexDump(const unsigned char *buf, int size) {
    std::string result;
    result.reserve(3 * size);
    for (int i = 0; i < size; ++i) {
        char digits[] = "0123456789ABCDEF";
        if (i != 0) {
            result += ' ';
        }
        result += digits[buf[i] / 16];
        result += digits[buf[i] % 16];
    }
    return result;
}

void dumpPosition(const char *fen) {
    using namespace ChessIntf;

    if (fen[0] == '\0' || fen[0] == '#') {
        return;
    }

    Board board;
    MoveList moves;
    boardFromFen(board, fen);
    std::cout << "fen: " << fen << std::endl;
#ifdef RUN_SELF_TESTS
    selfTest(board);
#endif

    generateMoves(board, moves);
    std::vector<std::string> moveList = getMoveStrList(board, moves);

    std::cout << "moves: [" << "\n";
    for (const std::string &str : moveList) {
        std::cout << "  " << str << "\n";
    }
    std::cout << "]\n";
    for (bool color : {true, false}) {
        std::cout << (color ? "white" : "black") << "-heatmap: [\n";
        for (char y = '8'; y >= '1'; --y) {
            std::cout << "  ";
            for (char x = 'a'; x <= 'h'; ++x) {
                std::cout << (isAttacked(board, color, x, y) ? "#" : ".");
            }
            std::cout << "\n";
        }
        std::cout << "]\n";
    }

#ifdef RUN_SELF_TESTS
    for (int i = 0; i < getMoveCount(moves); ++i) {
        const Move &move = getMove(moves, i);
        MovePersistence persistence;
        makeMove(board, move, persistence);
        if (!isOpponentKingAttacked(board)) {
            selfTest(board);
        }
        unmakeMove(board, move, persistence);
    }
#endif

    uint64_t hsh = 0;
    std::string moveChain;
    moveChain.clear();
    depthDump(board, hsh, 1, true, moveChain);
    std::cout << "depth-dump-at-1-heatmaps: " << hsh << "\n";

    hsh = 0;
    moveChain.clear();
    depthDump(board, hsh, 2, false, moveChain);
    std::cout << "depth-dump-at-2: " << hsh << "\n";

#ifdef DEPTH_DUMP_LARGE
    hsh = 0;
    moveChain.clear();
    depthDump(board, hsh, 2, true, moveChain);
    std::cout << "depth-dump-at-2-heatmaps: " << hsh << "\n";

    hsh = 0;
    moveChain.clear();
    depthDump(board, hsh, 3, false, moveChain);
    std::cout << "depth-dump-at-3: " << hsh << "\n";
#endif

    std::cout << "\n";
}

int main() {
    std::ios_base::sync_with_stdio(false);
    ChessIntf::init();

    std::cout << "Testing " << ChessIntf::getImplName() << "...\n\n";

    std::ifstream fens("../boards.fen");
    if (!fens) {
        std::cerr << "Cannot locate file ../boards.fen; aborting" << std::endl;
        return 1;
    }

    std::string fen;
    while (getline(fens, fen)) {
        dumpPosition(fen.c_str());
    }

    return 0;
}
