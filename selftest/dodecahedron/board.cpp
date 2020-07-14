#include "board.h"
#include "cpputil.h"
#include "movegen.h"

void recalc_board(BOARD& b)
{
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 7; j++)
        {
            b.piece_count[i][j] = 0;
            b.piece_bit[i][j] = 0;
        }
    b.col_piece[0] = 0;
    b.col_piece[1] = 0;
    for (int i = 0; i < 64; i++) b.list_pos[i] = 0;
    for (int i = 0; i < 64; i++)
    {
        int k = get_kind(b.board[i]),
            c = get_color(b.board[i]);
        if (get_kind(k) == NONE) continue;
        #define cnt b.piece_count[c][k]
        b.pieces[c][k][cnt] = i;
        b.list_pos[i] = cnt;
        cnt++;
        #undef cnt
        b.piece_bit[c][k] |= getbit(i);
        b.col_piece[c] |= getbit(i);
    }
    b.all_piece = b.col_piece[0] | b.col_piece[1];
}

char piece_to_char(PIECE p)
{
    int k = get_kind(p), c = get_color(p);
    bool is_white = (c == WHITE);
    switch (k)
    {
        case NONE  : return '.'; break;
        case PAWN  : return is_white ? 'P' : 'p'; break;
        case KNIGHT: return is_white ? 'N' : 'n'; break;
        case BISHOP: return is_white ? 'B' : 'b'; break;
        case ROOK  : return is_white ? 'R' : 'r'; break;
        case QUEEN : return is_white ? 'Q' : 'q'; break;
        case KING  : return is_white ? 'K' : 'k'; break;
    }
    return '?';
}

void out_board(const BOARD& b)
{
    for (int i = 0; i < 64; i++)
    {
        out_char(piece_to_char(b.board[i]));
        if ((i & 7) == 7) out_char('\n');
    }
    if (b.castling[WHITE][SHORT]) out_char('K');
    if (b.castling[WHITE][LONG] ) out_char('Q');
    if (b.castling[BLACK][SHORT]) out_char('k');
    if (b.castling[BLACK][LONG] ) out_char('q');
    out_str(" enpassant=");
    out_char((b.enpassant_line < 0) ? '-' : ('a' + b.enpassant_line));
    out_str(" moves=");
    out_str((b.move_side == WHITE) ? "WHITE" : "BLACK");
    out_str(" movectr="); out_int(b.move_counter);
    out_char('\n');
}

bool internal_load_from_fen(BOARD& b, const char* fen) // Parsing FEN without initialization!
{
    // 1. Board
    int x = 0, y = 0;
    while (*fen)
    {
        bool loop_end = false;
        switch (*fen)
        {
            // pieces
            #define put_piece(color, kind) \
                if (x > 7 || y > 7) return false; \
                b.board[arrpos(x, y)] = make_piece(color, kind); \
                y++;
            case 'p': put_piece(BLACK, PAWN); break;
            case 'P': put_piece(WHITE, PAWN); break;
            case 'n': put_piece(BLACK, KNIGHT); break;
            case 'N': put_piece(WHITE, KNIGHT); break;
            case 'b': put_piece(BLACK, BISHOP); break;
            case 'B': put_piece(WHITE, BISHOP); break;
            case 'r': put_piece(BLACK, ROOK); break;
            case 'R': put_piece(WHITE, ROOK); break;
            case 'q': put_piece(BLACK, QUEEN); break;
            case 'Q': put_piece(WHITE, QUEEN); break;
            case 'k': put_piece(BLACK, KING); break;
            case 'K': put_piece(WHITE, KING); break;
            #undef put_piece
            // spaces
            case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8':
                y += ((*fen) - '0');
            break;
            case '/': x++; y = 0; break;
            // others
            case ' ': loop_end = true; break;
            default: return false; break;
            }
        fen++;
        if (loop_end) break;
    }
    if (!(*fen)) return false;
    // 2. Move side
    switch (*fen)
    {
        case 'w': b.move_side = WHITE; break;
        case 'b': b.move_side = BLACK; break;
        default: return false; break;
    }
    fen++;
    if ((*fen) != ' ') return false;
    fen++;
    if (!(*fen)) return false;
    // 3. Castling
    while (*fen)
    {
        bool loop_end = false;
        switch (*fen)
        {
            // castling marks
            case 'K': b.castling[WHITE][SHORT] = true; break;
            case 'k': b.castling[BLACK][SHORT] = true; break;
            case 'Q': b.castling[WHITE][LONG ] = true; break;
            case 'q': b.castling[BLACK][LONG ] = true; break;
            // others
            case '-': /* skip */ break;
            case ' ': loop_end = true; break;
            default: return false; break;
        }
        fen++;
        if (loop_end) break;
    }
    if (!(*fen)) return false;
    // 4. En passant
    switch (*fen)
    {
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
            b.enpassant_line = (*fen) - 'a';
            fen++;
            if (!(*fen)) return false;
        break;
        case '-': b.enpassant_line = -1; break;
        default: return false; break;
    }
    fen++;
    if (!(*fen)) return false;
    fen++;
    // 5. Move counter
    b.move_counter = 0;
    while (*fen)
    {
        if ('0' <= (*fen) && (*fen) <= '9')
            b.move_counter = b.move_counter * 10 + ((*fen) - '0');
        else return true;
        fen++;
    }
    // That's all! :)
    return true;
}

bool load_from_fen(BOARD& b, const char* fen)
{
    b = clear_board();
    bool res = internal_load_from_fen(b, fen);
    if (res) res = validate_board(b);
    return res;
}

void save_to_fen(const BOARD& b, char* fen)
{
    // 1. Board
    for (int i = 0; i < 8; i++)
    {
        char space_cnt = '0';
        #define push_space \
            if (space_cnt != '0') \
            { \
                *(fen++) = space_cnt; \
                space_cnt = '0'; \
            }
        for (int j = 0; j < 8; j++)
        {
            if (b.board[arrpos(i, j)] == NONE)
                space_cnt++;
            else
            {
                push_space;
                *(fen++) = piece_to_char(b.board[arrpos(i, j)]);
            }
        }
        push_space;
        if (i < 7) *(fen++) = '/';
        #undef push_space
    }
    // 2. Move side
    *(fen++) = ' ';
    *(fen++) = (b.move_side == WHITE) ? 'w' : 'b';
    // 3. Castling
    *(fen++) = ' ';
    bool has_castling = false;
    if (b.castling[WHITE][SHORT]) { has_castling = true; *(fen++) = 'K'; }
    if (b.castling[WHITE][LONG ]) { has_castling = true; *(fen++) = 'Q'; }
    if (b.castling[BLACK][SHORT]) { has_castling = true; *(fen++) = 'k'; }
    if (b.castling[BLACK][LONG ]) { has_castling = true; *(fen++) = 'q'; }
    if (!has_castling) *(fen++) = '-';
    // 4. En passant
    *(fen++) = ' ';
    if (b.enpassant_line < 0)
        *(fen++) = '-';
    else
    {
        *(fen++) = 'a' + b.enpassant_line;
        *(fen++) = (b.move_side == WHITE) ? '6' : '3';
    }
    // 5. Move counter
    *(fen++) = ' ';
    fen = int_to_str(b.move_counter, fen);
    // 6. Move number (always put to 1!)
    *(fen++) = ' ';
    *(fen++) = '1';
    // That's all! :)
    *fen = 0;
}

bool validate_board(BOARD& b)
{
    const BITBOARD wrong_pawn_pos =
        string_to_bitboard(
            "11111111"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "00000000"
            "11111111"
        );
    // First, recalc it.
    recalc_board(b);
    // no kings, too many kings.
    if (b.piece_count[WHITE][KING] != 1 || b.piece_count[BLACK][KING] != 1) return false;
    // too many pieces.
    if (popcount(b.col_piece[WHITE]) > 16 || popcount(b.col_piece[BLACK]) > 16) return false;
    // illegal pawn position
    if ((b.piece_bit[WHITE][PAWN] | b.piece_bit[BLACK][PAWN]) & wrong_pawn_pos) return false;
    // opponent king attacked
    if (is_opponent_king_attacked(b)) return false;
    // Updating AllowCastling
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            if (b.board[arrpos(castling_rows[i], castling_king_col)] != make_piece(i, KING) ||
                b.board[arrpos(castling_rows[i], castling_rook_cols[j])]  != make_piece(i, ROOK))
            { b.castling[i][j] = false; }
    // Updating enpassant
    if (b.enpassant_line >= 0)
    {
        if (b.board[arrpos(enpassant_row[b.move_side], b.enpassant_line)] != make_piece(!b.move_side, PAWN))
        { b.enpassant_line = -1; }
    }
    // That's all :)
    return true;
}

BOARD start_position()
{
    BOARD b;
    PIECE pieces[8] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};
    for (int i = 0; i < 64; i++) b.board[i] = NONE;
    for (int i = 0; i < 8; i++)
    {
        b.board[arrpos(0, i)] = make_piece(BLACK, pieces[i]);
        b.board[arrpos(1, i)] = make_piece(BLACK, PAWN);
        b.board[arrpos(6, i)] = make_piece(WHITE, PAWN);
        b.board[arrpos(7, i)] = make_piece(WHITE, pieces[i]);
    }
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            b.castling[i][j] = true;
    b.enpassant_line = -1;
    b.move_side = WHITE;
    b.move_counter = 0;
    recalc_board(b);
    return b;
}

BOARD clear_board()
{
    BOARD b;
    for (int i = 0; i < 64; i++) b.board[i] = NONE;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            b.castling[i][j] = false;
    b.enpassant_line = -1;
    b.move_side = WHITE;
    b.move_counter = 0;
    recalc_board(b);
    return b;
}

bool board_ok(const BOARD& b) // Check if it's OK.
{
    // 1. Checking b.piece_count, b.piece_bit, b.col_piece, b.all_piece.
    int _piece_count[COLOR_ARRAY_SIZE][PIECE_ARRAY_SIZE];
    BITBOARD _piece_bit[COLOR_ARRAY_SIZE][PIECE_ARRAY_SIZE];
    BITBOARD _col_piece[COLOR_ARRAY_SIZE] = {0, 0};
    BITBOARD _all_piece = 0;
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 7; j++)
        {
            _piece_count[i][j] = 0;
            _piece_bit[i][j] = 0;
        }
    for (int i = 0; i < 64; i++)
    {
        int k = get_kind(b.board[i]),
            c = get_color(b.board[i]);
        if (get_kind(k) == NONE) continue;
        _piece_count[c][k]++;
        _piece_bit[c][k] |= getbit(i);
        _col_piece[c] |= getbit(i);
        _all_piece |= getbit(i);
    }
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 7; j++)
            if ((b.piece_count[i][j] != _piece_count[i][j]) ||
                (b.piece_bit  [i][j] != _piece_bit  [i][j]))
                return false;
    for (int i = 0; i < 2; i++)
        if (b.col_piece[i] != _col_piece[i]) return false;
    if (b.all_piece != _all_piece) return false;
    // 2. Checking pieces & list_pos
    for (int i = 0; i < 64; i++)
    {
        int k = get_kind(b.board[i]),
            c = get_color(b.board[i]);
        if (get_kind(k) == NONE) continue;
        if (b.list_pos[i] >= b.piece_count[c][k]) return false;
        if (b.pieces[c][k][b.list_pos[i]] != i) return false;
    }
    // Everything is OK :)
    return true;
}
