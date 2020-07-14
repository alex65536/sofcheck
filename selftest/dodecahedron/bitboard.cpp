#define NO_USE_EXTERN
    #include "bitboard.h"
#undef NO_USE_EXTERN

#include "cpputil.h"

BITBOARD string_to_bitboard(const char* s)
{
    BITBOARD b = 0;
    for (int i = 0; *s; i++, s++) b |= BITBOARD(*s == '1') << i;
    return b;
}

void out_bitboard(BITBOARD b)
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++) out_char((b & bitpos(i, j)) ? '1' : '0');
        out_char('\n');
    }
}

bool gen_bitboards()
{
    // pawn_promote_from
    pawn_promote_from[WHITE] =
        string_to_bitboard(
            "00000000"
            "11111111"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000");
    pawn_promote_from[BLACK] =
        string_to_bitboard(
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "11111111"
            "00000000");
    // pawn_single & pawn_double
    #define bit_push(var) (((var) < 0) ? 0 : getbit(var))
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        {
            int _w1 = -1, _w2 = -1, _b1 = -1, _b2 = -1;
            if (i != 0) _w1 = arrpos(i-1, j);
            if (i == 6) _w2 = arrpos(i-2, j);
            if (i != 7) _b1 = arrpos(i+1, j);
            if (i == 1) _b2 = arrpos(i+2, j);
            pawn_single[WHITE][arrpos(i, j)] = bit_push(_w1);
            pawn_double[WHITE][arrpos(i, j)] = bit_push(_w2);
            pawn_single[BLACK][arrpos(i, j)] = bit_push(_b1);
            pawn_double[BLACK][arrpos(i, j)] = bit_push(_b2);

            pawn_single_move[WHITE][arrpos(i, j)] = _w1;
            pawn_double_move[WHITE][arrpos(i, j)] = _w2;
            pawn_single_move[BLACK][arrpos(i, j)] = _b1;
            pawn_double_move[BLACK][arrpos(i, j)] = _b2;
        }
    // pawn_eat & pawn_attacked_by
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        {
            int _wl = -1, _wr = -1, _bl = -1, _br = -1;
            if (i != 0)
            {
                if (j != 0) _wl = arrpos(i-1, j-1);
                if (j != 7) _wr = arrpos(i-1, j+1);
            }
            if (i != 7)
            {
                if (j != 0) _bl = arrpos(i+1, j-1);
                if (j != 7) _br = arrpos(i+1, j+1);
            }
            pawn_eat_left[WHITE][arrpos(i, j)] = bit_push(_wl);
            pawn_eat_right[WHITE][arrpos(i, j)] = bit_push(_wr);
            pawn_eat_left[BLACK][arrpos(i, j)] = bit_push(_bl);
            pawn_eat_right[BLACK][arrpos(i, j)] = bit_push(_br);

            pawn_eat_left_move[WHITE][arrpos(i, j)] = _wl;
            pawn_eat_right_move[WHITE][arrpos(i, j)] = _wr;
            pawn_eat_left_move[BLACK][arrpos(i, j)] = _bl;
            pawn_eat_right_move[BLACK][arrpos(i, j)] = _br;

            pawn_attacked_by[WHITE][arrpos(i, j)] = bit_push(_bl) | bit_push(_br);
            pawn_attacked_by[BLACK][arrpos(i, j)] = bit_push(_wl) | bit_push(_wr);
        }
    // pawn_enpassant
    for (int i = 0; i < 8; i++)
    {
        BITBOARD _wsrc = 0, _bsrc = 0;
        if (i != 0)
        {
            _wsrc |= bitpos(3, i-1);
            _bsrc |= bitpos(4, i-1);
        }
        if (i != 7)
        {
            _wsrc |= bitpos(3, i+1);
            _bsrc |= bitpos(4, i+1);
        }
        pawn_enpassant_src[i][WHITE] = _wsrc;
        pawn_enpassant_src[i][BLACK] = _bsrc;
        pawn_enpassant[i][WHITE] = bitpos(3, i);
        pawn_enpassant[i][BLACK] = bitpos(4, i);
        pawn_enpassant_move[i][WHITE] = arrpos(2, i);
        pawn_enpassant_move[i][BLACK] = arrpos(5, i);
    }
    #undef bit_push
    // knight
    const int knight_x[BOARD_ARRAY_SIZE] = {-2, -2,  2,  2, -1, -1,  1,  1};
    const int knight_y[BOARD_ARRAY_SIZE] = {-1,  1, -1,  1, -2,  2, -2,  2};
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        {
            BITBOARD b = 0;
            int cnt = 0;
            for (int k = 0; k < 8; k++)
            {
                int x = i + knight_x[k], y = j + knight_y[k];
                if (x >= 0 && x < 8 && y >= 0 && y < 8)
                {
                    knight_moves[arrpos(i, j)][cnt++] = arrpos(x, y);
                    b |= bitpos(x, y);
                }
            }
            knight_move_count[arrpos(i, j)] = cnt;
            knight[arrpos(i, j)] = b;
        }
    // bishops, rooks, queens
    const int dir_x[BOARD_ARRAY_SIZE] = {-1, -1, -1,  0,  0,  1,  1,  1};
    const int dir_y[BOARD_ARRAY_SIZE] = {-1,  0,  1, -1,  1, -1,  0,  1};
    for (int k = 0; k < 8; k++)
        for (int i = 0; i < 8; i++)
            for (int j = 0; j < 8; j++)
            {
                BITBOARD b = 0;
                int cnt = 0;
                int x = i + dir_x[k], y = j + dir_y[k];
                while (x >= 0 && x < 8 && y >= 0 && y < 8)
                {
                    b |= bitpos(x, y);
                    dir_val[k][arrpos(i, j)][cnt] = b;
                    dir_cell[k][arrpos(i, j)][cnt] = bitpos(x, y);
                    dir_moves[k][arrpos(i, j)][cnt] = arrpos(x, y);
                    cnt++;
                    x += dir_x[k], y += dir_y[k];
                }
                dir_move_count[k][arrpos(i, j)] = cnt;
                while (cnt < DIR_COUNT)
                {
                    dir_val[k][arrpos(i, j)][cnt] = b;
                    dir_cell[k][arrpos(i, j)][cnt] = 0;
                    cnt++;
                }
            }
    // kings
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++)
        {
            BITBOARD b = 0;
            int cnt = 0;
            for (int k = 0; k < 8; k++)
            {
                int x = i + dir_x[k], y = j + dir_y[k];
                if (x >= 0 && x < 8 && y >= 0 && y < 8)
                {
                    king_moves[arrpos(i, j)][cnt++] = arrpos(x, y);
                    b |= bitpos(x, y);
                }
            }
            king_move_count[arrpos(i, j)] = cnt;
            king[arrpos(i, j)] = b;
        }
    // free_castling
    free_castling[WHITE][SHORT] =
        string_to_bitboard(
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000110");
    free_castling[WHITE][LONG] =
        string_to_bitboard(
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "01110000");
    free_castling[BLACK][SHORT] =
        string_to_bitboard(
            "00000110"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000");
    free_castling[BLACK][LONG] =
        string_to_bitboard(
            "01110000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000");
    return true;
}
bool bitboards_generated = gen_bitboards();
