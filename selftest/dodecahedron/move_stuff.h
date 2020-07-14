#define make_move_hash(move) \
    MOVE_PERSISTENCE p; \
    int cl[6], cls; \
    POS_COST new_cost = dyn_cost; \
    ZOBRIST new_hsh = hsh; \
    cls = get_changes_list(brd, move, cl); \
    dynamic_calc_before_move(brd, new_cost, cl, cls); \
    make_move_recalc_hash(brd, move, p, new_hsh, cl, cls); \
    dynamic_calc_after_move(brd, new_cost, cl, cls); \
    hashtable_push(htab, new_hsh);

#define make_move_no_hash(move) \
    MOVE_PERSISTENCE p; \
    int cl[6], cls; \
    POS_COST new_cost = dyn_cost; \
    cls = get_changes_list(brd, move, cl); \
    dynamic_calc_before_move(brd, new_cost, cl, cls); \
    make_move(brd, move, p); \
    dynamic_calc_after_move(brd, new_cost, cl, cls);

#define unmake_move_hash(move) \
    unmake_move(brd, move, p); \
    hashtable_pop(htab, new_hsh);

#define unmake_move_no_hash(move) \
    unmake_move(brd, move, p);
