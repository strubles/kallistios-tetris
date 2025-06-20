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

void dbg_print_controller(InputState* input) {
    dbglog(DBG_INFO, "Up\nPressed: %d, Just pressed: %d, Just released: %d" \
        "\nDown\nPressed: %d, Just pressed: %d, Just released: %d" \
        "\nLeft\nPressed: %d, Just pressed: %d, Just released: %d" \
        "\nRight\nPressed: %d, Just pressed: %d, Just released: %d\n",
        input->dpad_up.pressed, input->dpad_up.just_pressed, input->dpad_up.just_released,
        input->dpad_down.pressed, input->dpad_down.just_pressed, input->dpad_down.just_released,
        input->dpad_left.pressed, input->dpad_left.just_pressed, input->dpad_left.just_released,
        input->dpad_right.pressed, input->dpad_right.just_pressed, input->dpad_right.just_released
    );
}
