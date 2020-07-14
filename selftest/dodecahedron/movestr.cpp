#include "movestr.h"
#include "cpputil.h"
#include "board.h"

void cell_to_str(int cell, char* str)
{
    str[0] = 'a' + cell_y(cell);
    str[1] = '8' - cell_x(cell);
    str[2] = 0;
}

void out_cell(int cell)
{
    char str[3];
    cell_to_str(cell, str);
    out_str(str);
}

void move_to_str(const MOVE& m, char* str)
{
    cell_to_str(m.src, str); str += 2;
    cell_to_str(m.dst, str); str += 2;
    switch (get_kind(m.promote))
    {
        case PAWN:   *(str++) = 'p'; break;
        case KNIGHT: *(str++) = 'n'; break;
        case BISHOP: *(str++) = 'b'; break;
        case ROOK  : *(str++) = 'r'; break;
        case QUEEN : *(str++) = 'q'; break;
        case KING  : *(str++) = 'k'; break;
    }
    (*str) = 0;
}

void out_move(const MOVE& m)
{
    char str[7];
    move_to_str(m, str);
    out_str(str);
}

inline bool move_fits(const MOVE& m, const char* str)
{
    int len = strlen(str);
    // Checking length
    if (len < 4) return false;
    // Checking dst and src
    if (str[0] - 'a' != cell_y(m.src)) return false;
    if ('8' - str[1] != cell_x(m.src)) return false;
    if (str[2] - 'a' != cell_y(m.dst)) return false;
    if ('8' - str[3] != cell_x(m.dst)) return false;
    // Checking promote
    if (!m.promote) return true;
    if (len < 5) return false;
    switch (get_kind(m.promote))
    {
        case KNIGHT: return (str[4] == 'n'); break;
        case BISHOP: return (str[4] == 'b'); break;
        case ROOK:   return (str[4] == 'r'); break;
        case QUEEN:  return (str[4] == 'q'); break;
    }
    fatal_error("move_fits error: such move doesn't exist");
    return false;
}

MOVE* parse_move(MOVE* lst, const char* move_str)
{
    for (; lst->flags != FLAG_END_OF_LIST; lst++)
        if (move_fits(*lst, move_str)) return lst;
    return NULL;
}
