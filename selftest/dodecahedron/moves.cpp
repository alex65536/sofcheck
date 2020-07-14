#include "moves.h"
#include "cpputil.h"

#define enpassant_eat_cell(move_side, move) (((move_side) == WHITE) ? (move).dst + 8 : (move).dst - 8)

int get_changes_list(const BOARD& b, const MOVE& m, int* change_list)
{
    switch (m.flags)
    {
        case FLAG_NONE:
            change_list[0] = m.src;
            change_list[1] = m.dst;
            return 2;
        break;
        case FLAG_ENPASSANT:
            change_list[0] = m.src;
            change_list[1] = m.dst;
            change_list[2] = enpassant_eat_cell(b.move_side, m);
            return 3;
        break;
        case FLAG_SHORT_CASTLING:
            change_list[0] = m.src;
            change_list[1] = m.dst;
            change_list[2] = arrpos(castling_rows[b.move_side],  castling_tmp_short_col);
            change_list[3] = arrpos(castling_rows[b.move_side], castling_rook_short_col);
            return 4;
        break;
        case FLAG_LONG_CASTLING:
            change_list[0] = m.src;
            change_list[1] = m.dst;
            change_list[2] = arrpos(castling_rows[b.move_side],  castling_tmp_long_col);
            change_list[3] = arrpos(castling_rows[b.move_side], castling_rook_long_col);
            return 4;
        break;
        case FLAG_NULL_MOVE:
            return 0;
        break;
    }
    fatal_error("get_changes_list error: bad move");
    return 0;
}

inline void del_val_no_change_board(BOARD& b, int cell)
{
    if (!b.board[cell]) return; // we won't clear an empty cell!
    int k = get_kind(b.board[cell]),
        c = get_color(b.board[cell]);
    // recalc pieces & list_pos
    #define cnt b.piece_count[c][k]
    int lp = b.list_pos[cell];
    if (lp != cnt-1)
    {
        b.pieces[c][k][lp] = b.pieces[c][k][cnt-1];
        b.list_pos[b.pieces[c][k][lp]] = lp;
    }
    cnt--;
    #undef cnt
    // recalc bitboards
    b.all_piece ^= getbit(cell);
    b.col_piece[c] ^= getbit(cell);
    b.piece_bit[c][k] ^= getbit(cell);
}

inline void push_val_no_change_board(BOARD& b, int cell, PIECE new_val) // ONLY to an empty cell
{
    if (!new_val) return; // we won't push an empty value!
    int k = get_kind(new_val),
        c = get_color(new_val);
    // recalc pieces & list_pos
    #define cnt b.piece_count[c][k]
    b.pieces[c][k][cnt] = cell;
    b.list_pos[cell] = cnt;
    cnt++;
    #undef cnt
    // recalc bitboards
    b.all_piece |= getbit(cell);
    b.col_piece[c] |= getbit(cell);
    b.piece_bit[c][k] |= getbit(cell);
}

inline void change_piece(BOARD& b, int cell, PIECE new_val)
{
    del_val_no_change_board(b, cell);
    push_val_no_change_board(b, cell, new_val);
    b.board[cell] = new_val;
}

inline void save_state(BOARD& b, MOVE_PERSISTENCE& p)
{
    memcpy(p.was_castling, b.castling, sizeof(bool) * COLOR_ARRAY_SIZE * CASTLING_ARRAY_SIZE);
    p.was_enpassant_line = b.enpassant_line;
    p.was_move_counter = b.move_counter;
}

inline void load_state(BOARD& b, const MOVE_PERSISTENCE& p)
{
    memcpy(b.castling, p.was_castling, sizeof(bool) * COLOR_ARRAY_SIZE * CASTLING_ARRAY_SIZE);
    b.enpassant_line = p.was_enpassant_line;
    b.move_counter = p.was_move_counter;
}

void make_move(BOARD& b, const MOVE& m, MOVE_PERSISTENCE& p)
{
    save_state(b, p);
    bool null_ctr = false;
    switch (m.flags)
    {
        case FLAG_NONE:
            p.was_piece = b.board[m.dst];
            // update null_ctr
            null_ctr = (get_kind(b.board[m.src]) == PAWN || b.board[m.dst] != 0);
            // update board
            change_piece(b, m.dst, m.promote ? m.promote : b.board[m.src]);
            change_piece(b, m.src, 0);
            // update castling
            for (int i = 0; i < 2; i++)
            {
                int short_rook_pos = arrpos(castling_rows[i], castling_rook_short_col),
                    long_rook_pos  = arrpos(castling_rows[i], castling_rook_long_col),
                    king_pos       = arrpos(castling_rows[i], castling_king_col);

                if (m.dst == short_rook_pos || m.src == short_rook_pos)
                    b.castling[i][SHORT] = false;
                if (m.dst == long_rook_pos || m.src == long_rook_pos)
                    b.castling[i][LONG] = false;
                if (m.dst == king_pos || m.src == king_pos)
                    b.castling[i][SHORT] = b.castling[i][LONG] = false;
            }
        break;
        case FLAG_ENPASSANT:
            // update null_ctr
            null_ctr = true;
            // update board
            change_piece(b, m.dst, b.board[m.src]);
            change_piece(b, m.src, 0);
            change_piece(b, enpassant_eat_cell(b.move_side, m), 0);
        break;
        case FLAG_SHORT_CASTLING:
            change_piece(b, m.dst, b.board[m.src]);
            change_piece(b, m.src, 0);
            change_piece(b, arrpos(castling_rows[b.move_side],  castling_tmp_short_col), make_piece(b.move_side, ROOK));
            change_piece(b, arrpos(castling_rows[b.move_side], castling_rook_short_col), 0);
            // update castling
            b.castling[b.move_side][SHORT] = b.castling[b.move_side][LONG] = false;
        break;
        case FLAG_LONG_CASTLING:
            change_piece(b, m.dst, b.board[m.src]);
            change_piece(b, m.src, 0);
            change_piece(b, arrpos(castling_rows[b.move_side], castling_tmp_long_col),  make_piece(b.move_side, ROOK));
            change_piece(b, arrpos(castling_rows[b.move_side], castling_rook_long_col), 0);
            // update castling
            b.castling[b.move_side][SHORT] = b.castling[b.move_side][LONG] = false;
        break;
        case FLAG_NULL_MOVE:
            // do nothing
        break;
    }
    // update enpassant
    if (m.double_move)
        b.enpassant_line = cell_y(m.dst);
    else
        b.enpassant_line = -1;
    // update the rest of the stuff
    b.move_side = !b.move_side;
    if (null_ctr)
        b.move_counter = 0;
    else
        b.move_counter++;
}

void unmake_move(BOARD& b, const MOVE& m, const MOVE_PERSISTENCE& p)
{
    load_state(b, p);
    b.move_side = !b.move_side;
    switch (m.flags)
    {
        case FLAG_NONE:
            change_piece(b, m.src, m.promote ? make_piece(b.move_side, PAWN) : b.board[m.dst]);
            change_piece(b, m.dst, p.was_piece);
        break;
        case FLAG_ENPASSANT:
            change_piece(b, m.src, b.board[m.dst]);
            change_piece(b, m.dst, 0);
            change_piece(b, enpassant_eat_cell(b.move_side, m), make_piece(!b.move_side, PAWN));
        break;
        case FLAG_SHORT_CASTLING:
            change_piece(b, m.src, b.board[m.dst]);
            change_piece(b, m.dst, 0);
            change_piece(b, arrpos(castling_rows[b.move_side],  castling_tmp_short_col), 0);
            change_piece(b, arrpos(castling_rows[b.move_side], castling_rook_short_col), make_piece(b.move_side, ROOK));
        break;
        case FLAG_LONG_CASTLING:
            change_piece(b, m.src, b.board[m.dst]);
            change_piece(b, m.dst, 0);
            change_piece(b, arrpos(castling_rows[b.move_side],  castling_tmp_long_col), 0);
            change_piece(b, arrpos(castling_rows[b.move_side], castling_rook_long_col), make_piece(b.move_side, ROOK));
        break;
        case FLAG_NULL_MOVE:
            // do nothing
        break;
    }
}
