#ifndef DODECAHEDRON_INTF_H_INCLUDED
#define DODECAHEDRON_INTF_H_INCLUDED

#include "board.h"
#include "moves.h"
#include "movestr.h"
#include "movegen.h"

namespace ChessIntf {
    
// This one doesn't support self-tests
// #define CHESS_INTF_HAS_SELF_TESTS

using Board = BOARD;
using Move = MOVE;
using MovePersistence = MOVE_PERSISTENCE;

struct MoveList {
    Move moves[240];
    int count;
};

inline const char *getImplName() {
    return "Dodecahedron";
}

inline const Move &getMove(const MoveList &lst, int idx) {
    return lst.moves[idx];
}

inline int getMoveCount(const MoveList &lst) {
    return lst.count;
}

inline void init() {}

inline void boardFromFen(Board &board, const char *fen) {
    load_from_fen(board, fen);
}

inline void makeMove(Board &board, const Move &move, MovePersistence &p) {
    make_move(board, move, p);
}

inline void unmakeMove(Board &board, const Move &move, MovePersistence &p) {
    unmake_move(board, move, p);
}

inline void moveStr(const Board &, const Move &move, char *str) {
    move_to_str(move, str);
}

inline void generateMoves(const Board &board, MoveList &moves) {
    moves.count = gen_moves(board, moves.moves);
}

inline bool isAttacked(const Board &board, bool isWhite, char cx, char cy) {
    return is_attacked(board, isWhite ? WHITE : BLACK, arrpos('8' - cy, cx - 'a'));
}

inline bool isOpponentKingAttacked(const Board &board) {
    return is_opponent_king_attacked(board);
}

}

#endif

