#include <kos/dbglog.h>
#include "types.h"

// Debug log functions

void dbg_print_bag(GameInstance* game){
    dbglog(DBG_INFO, "Bag: [");

    for (int i=0; i<7; i++) {
        dbglog(DBG_INFO, "%d, ", game->bag[i]);
    }
    dbglog(DBG_INFO, "]\nCurrent index: %d (tetromino num: %d)\n", game->bag_index, game->bag[game->bag_index]);
}
