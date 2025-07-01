#include <kos/dbglog.h>

#include <types.h>
#include <constants.h>

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

void dbg_print_field(GameInstance *game) {
    dbglog(DBG_INFO, "\n== FIELD + ACTIVE TETRO OVERLAY ==\n");

    for (int row = 0; row < 24; row++) {
        dbglog(DBG_INFO, "%02d | ", row);
        for (int col = 0; col < 12; col++) {
            int draw_char = -1;

            // Check if this cell is part of the active tetromino
            int size = game->active_tetro.info->size;
            for (int dy = 0; dy < size; dy++) {
                for (int dx = 0; dx < size; dx++) {
                    if (game->active_tetro.dummy[dy][dx] != BLOCK_NONE) {
                        int abs_y = game->active_tetro.top_y + dy;
                        int abs_x = game->active_tetro.left_x + dx;
                        if (abs_y == row && abs_x == col) {
                            draw_char = game->active_tetro.dummy[dy][dx]; // Overlay active block
                        }
                    }
                }
            }

            if (draw_char >= 0) {
                dbglog(DBG_INFO, "@");  // Active tetromino block
            } else {
                BlockColor block = game->field[row][col];
                if (block == BLOCK_NONE) {
                    dbglog(DBG_INFO, ".");
                } else {
                    dbglog(DBG_INFO, "#");  // Committed block
                }
            }
        }
        dbglog(DBG_INFO, "\n");
    }

    dbglog(DBG_INFO, "===============================\n");
}
