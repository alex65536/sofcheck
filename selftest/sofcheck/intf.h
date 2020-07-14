#ifndef SOFCHECK_INTF_H_INCLUDED
#define SOFCHECK_INTF_H_INCLUDED

#include "core/init.h"
#include "core/board.h"
#include "core/move.h"
#include "core/movegen.h"
#include "core/strutil.h"
#include "core/test/selftest.h"

namespace ChessIntf {

#define CHESS_INTF_HAS_SELF_TESTS

using Board = SoFCore::Board;
using Move = SoFCore::Move;
using MovePersistence = SoFCore::MovePersistence;

struct MoveList {
    Move moves[240];
    int count;
};

inline const char *getImplName() {
    return "SoFCheck";
}

inline void selfTest(Board board) {
    SoFCore::Test::selfTest(board);
}

inline void init() {
    SoFCore::init();
}

inline const Move &getMove(const MoveList &lst, int idx) {
    return lst.moves[idx];
}

inline int getMoveCount(const MoveList &lst) {
    return lst.count;
}

inline void boardFromFen(Board &board, const char *fen) {
    board.setFromFen(fen);
}

inline void makeMove(Board &board, const Move &move, MovePersistence &p) {
    p = SoFCore::moveMake(board, move);
}

inline void unmakeMove(Board &board, const Move &move, MovePersistence &p) {
    SoFCore::moveUnmake(board, move, p);
}

inline void moveStr(const Board &, const Move &mv, char *str) {
    SoFCore::moveToStr(mv, str);
}

inline void generateMoves(const Board &board, MoveList &moves) {
    moves.count = static_cast<size_t>(SoFCore::genAllMoves(board, moves.moves));
}

inline bool isAttacked(const Board &board, bool isWhite, char cx, char cy) {
    using namespace SoFCore;
    coord_t coord = makeCoord(charToSubX(cy), charToSubY(cx));
    if (isWhite) {
        return isCellAttacked<Color::White>(board, coord);
    } else {
        return isCellAttacked<Color::Black>(board, coord);
    }
}

inline bool isOpponentKingAttacked(const Board &board) {
    return !isMoveLegal(board);
}

}

#endif


