#include <kos.h>

#include "types.h"
#include "tetromino.h"
#include "constants.h"
#include "main.h"

void init_new_tetro(GameInstance* game, TetrominoType type){
    // populates the active_tetro variable with data representing a newly spawned
    // tetromino of type 'type'.
    // It fills the "dummy" tetromino array with a fresh copy of that tetromino's
    // .shape array

    const TetrominoInfo *info = &tetromino_infos[type];

    game->active_tetro.type = type;
    game->active_tetro.left_x = info->initial_left_x;
    game->active_tetro.top_y = info->initial_top_y;
    game->active_tetro.info = info;
    game->active_tetro.orientation = DEFAULT;
    game->active_tetro.set = 0;
    game->ghost_tetro.dummy = game->active_tetro.dummy;

    // reset the holder to its initial state
    memset(game->active_tetro.dummy, 0, sizeof(game->active_tetro.dummy));
    for (int i=0; i<info->size; i++) {
        for (int j=0; j<info->size; j++) {
            game->active_tetro.dummy[i][j] = info->shape[i][j];
        }
    }

    game->hard_drop_distance = find_hard_drop_distance(game);
    update_ghost_piece(game);
}

void commit_active_tetro(GameInstance* game){
    // The active tetromino gets copied from the active tetromino array to the main field
    // data structure to "set" it.
    // THIS DOES NOT DO CHECKS to validate position! Check it first with check_valid_state()

    // dbglog(DBG_INFO, "commiting tetromino with left x %d, top y %d\n",game->active_tetro.left_x,game->active_tetro.top_y);

    int size = game->active_tetro.info->size;
    for (int relative_y = 0; relative_y < size; relative_y++) {
        for (int relative_x = 0; relative_x < size; relative_x++) {
            if (game->active_tetro.dummy[relative_y][relative_x] != COLOR_NONE) {

                // convert relative tetro array coordinates to absolute field coordinates
                int field_x = game->active_tetro.left_x + relative_x;
                int field_y = game->active_tetro.top_y + relative_y;

                game->field[field_y][field_x] = game->active_tetro.info->color;
            }
        }
    }
}

void tetro_left(GameInstance* game){
    game->active_tetro.left_x -= 1;
    if(!check_valid_state(game)){
        game->active_tetro.left_x += 1; //undo it
    } else {
        game->hard_drop_distance = find_hard_drop_distance(game);
        update_ghost_piece(game);
    }
}

void tetro_right(GameInstance* game){
    game->active_tetro.left_x += 1;
    if(!check_valid_state(game)){
        game->active_tetro.left_x -= 1; //undo it
    } else {
        game->hard_drop_distance = find_hard_drop_distance(game);
        update_ghost_piece(game);
    }
}

void update_ghost_piece(GameInstance* game){
    game->ghost_tetro.top_y = game->active_tetro.top_y + game->hard_drop_distance;
    game->ghost_tetro.left_x = game->active_tetro.left_x;
    // dbglog(DBG_INFO, "ghost tetro top y: %d, hard drop distance: %d\n", game->ghost_tetro.top_y, game->hard_drop_distance);
}

void soft_drop(GameInstance* game, int award_score, int commit){
    // one block at a time
    // dbglog(DBG_INFO, "tetro_fall: old tetro top y: %d\n", game->active_tetro.top_y);
    game->active_tetro.top_y += 1;
    // dbglog(DBG_INFO, "tetro_fall: new tetro top y: %d\n", game->active_tetro.top_y);

    if(!check_valid_state(game)){
        // dbglog(DBG_INFO, "%d is invalid\n", game->active_tetro.top_y);
        game->active_tetro.top_y -= 1; //undo it
        if (commit) {
            commit_active_tetro(game);
            game->active_tetro.set=1;
        }
    }
    else {
        if(award_score){
            game->score+=1;
        }
        game->hard_drop_distance = find_hard_drop_distance(game);//, game->active_tetro);
        update_ghost_piece(game);
    }
}

void hard_drop(GameInstance* game) {
    game->active_tetro.top_y += game->hard_drop_distance;
    commit_active_tetro(game);
    game->active_tetro.set=1;
}

int find_hard_drop_distance(GameInstance* game){
    int orig_y = game->active_tetro.top_y;

    int blocks_fallen = 0;

    while (1) {
        game->active_tetro.top_y += 1;
        if (!check_valid_state(game)) {
            game->active_tetro.top_y -= 1; // undo
            break;
        }
        else {
            blocks_fallen++;
        }
    }

    game->active_tetro.top_y = orig_y;
    return blocks_fallen;
}

void transpose_active_tetro(GameInstance* game){
    // Flips the array of the active tetromino along the diagonal (in place)
    // (for tetromino rotation)

    int n = game->active_tetro.info->size;
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            swap(&(game->active_tetro.dummy[i][j]), &(game->active_tetro.dummy[j][i]));
        }
    }
}

void reverse_active_tetro_row(GameInstance* game, int row) {
    // Reverse the given row in the active tetromino array (in place)
    // (for tetromino rotation)

    int n = game->active_tetro.info->size;

    // Reverse the elements in the specified row
    for (int i = 0; i < n / 2; i++) {
        int temp = game->active_tetro.dummy[row][i];
        game->active_tetro.dummy[row][i] = game->active_tetro.dummy[row][n - i - 1];
        game->active_tetro.dummy[row][n - i - 1] = temp;
    }
}

void update_orientation_cw(Tetrodata* tetro){
    tetro->orientation++;
    if(tetro->orientation>3){
        tetro->orientation=0;
    }
}

void update_orientation_ccw(Tetrodata* tetro){
    tetro->orientation--;
    if(tetro->orientation<0){
        tetro->orientation=3;
    }
}

void move_tetro_like_this(Tetrodata* tetro, int x, int y){
    tetro->left_x+=x;
    tetro->top_y+=y;
}

void swap(int* arg1, int* arg2)
// Swap the two values in memory (in place)
// Used in tetromino rotation
{
    int buffer = *arg1;
    *arg1 = *arg2;
    *arg2 = buffer;
}


void rotate_tetro_clockwise(GameInstance* game){
    if (game->active_tetro.type == TETRO_O) {
        return;
    }

    // first test is plain rotation, no offset

    // 1. transpose
    transpose_active_tetro(game);
    // 2. reverse rows
    for (int i=0; i < game->active_tetro.info->size; i++) {
        reverse_active_tetro_row(game, i);
    }

    if (check_valid_state(game)) {
        update_orientation_cw(&game->active_tetro);
        game->hard_drop_distance = find_hard_drop_distance(game);
        update_ghost_piece(game);
        return;
    }

    const TetrominoInfo *info = game->active_tetro.info;

    // If the basic rotation failed, iterate through the kick tests til we find a working one

    int rotation_type_index = game->active_tetro.orientation;

    for (int test_index = 0; test_index <= 3; test_index++) {

        move_tetro_like_this(
            &game->active_tetro,
            info->kicks_cw[rotation_type_index][test_index][0],
            info->kicks_cw[rotation_type_index][test_index][1]
        );

        if (check_valid_state(game)) {
            update_orientation_cw(&game->active_tetro);
            game->hard_drop_distance = find_hard_drop_distance(game);
            update_ghost_piece(game);
            return;
        }
    }

    // Never found a valid one; undo
    // (The last "test" is actually an "undo" offset to get back to the original location)

    move_tetro_like_this(&game->active_tetro,
                         info->kicks_cw[rotation_type_index][4][0],
                         info->kicks_cw[rotation_type_index][4][1]);

    for (int i=0; i < game->active_tetro.info->size; i++) {
        reverse_active_tetro_row(game, i);
    }
    transpose_active_tetro(game);
}

void rotate_tetro_counterclockwise(GameInstance* game){
    if (game->active_tetro.type == TETRO_O) {
        return;
    }

    // 1. reverse rows
    for (int i=0; i < game->active_tetro.info->size; i++) {
        reverse_active_tetro_row(game, i);
    }
    // 2. transpose
    transpose_active_tetro(game);

    if (check_valid_state(game)) {
        update_orientation_ccw(&game->active_tetro);
        game->hard_drop_distance = find_hard_drop_distance(game);
        update_ghost_piece(game);
        return;
    }

    const TetrominoInfo *info = game->active_tetro.info;
    int rotation_type_index = game->active_tetro.orientation;

    for (int test_index = 0; test_index <= 3; test_index++) {

        move_tetro_like_this(
            &game->active_tetro,
            info->kicks_ccw[rotation_type_index][test_index][0],
            info->kicks_ccw[rotation_type_index][test_index][1]
        );

        if (check_valid_state(game)) {
            update_orientation_ccw(&game->active_tetro);
            game->hard_drop_distance = find_hard_drop_distance(game);
            update_ghost_piece(game);
            return;
        }
    }

    move_tetro_like_this(&game->active_tetro,
                         info->kicks_ccw[rotation_type_index][4][0],
                         info->kicks_ccw[rotation_type_index][4][1]);

    transpose_active_tetro(game);
    for (int i=0; i < game->active_tetro.info->size; i++) {
        reverse_active_tetro_row(game, i);
    }
}

void hold_tetromino(GameInstance* game){

    const TetrominoInfo *tetromino_to_hold = game->active_tetro.info;
    
    if(game->held_tetro){ //if there's currently a tetromino already in the hold
        // swap it
        init_new_tetro(game, game->held_tetro->type);
        game->held_tetro = tetromino_to_hold;
    }
    else {
        //otherwise, make a new one
        game->held_tetro = tetromino_to_hold;
        generate_new_tetro(game);
    }
    game->hold_eligible=0;
}


int process_tetro_movement(GameInstance* game){
    // the triggers on the sega dreamcast are analog triggers that range from 0-255
    // I have the hold function trigger if it's at least half-pressed (128)
    if(game->input.trigger_left >= 128 && game->hold_eligible){
        hold_tetromino(game);
        game->hold_eligible=0;
    }

    if(game->input.dpad_up.just_pressed){
        hard_drop(game);
        game->move_timebuffer=10;
        return 0;
    }

    if(game->move_timebuffer<=0){
        if(game->input.dpad_down.pressed){
            // softdrop
            soft_drop(game, 1, 1);
            game->move_timebuffer=10;
        }
        if(game->input.dpad_left.pressed){
            tetro_left(game);
            game->move_timebuffer=10;
        }
        if(game->input.dpad_right.pressed){
            tetro_right(game);
            game->move_timebuffer=10;
        }
        
        if(game->input.button_y.just_pressed){
            rotate_tetro_clockwise(game);
        }
        if(game->input.button_x.just_pressed){
            rotate_tetro_counterclockwise(game);
        }
            
    }
    else {
            game->move_timebuffer-=1;
    }

    return 0;
}

void generate_new_tetro(GameInstance* game){
    TetrominoType random_id = game->bag[game->bag_index];

    game->bag_index++;
    if (game->bag_index==7) {
        game->bag_index=0;
        shuffle_bag(game);
    }

    init_new_tetro(game, random_id);

    if(!check_valid_state(game)){
        game->loss = 1;
    }
}

void process_tetro_fall(GameInstance *game) {
    float gravity = gravity_by_level[game->level];
    game->fall_timer += gravity;
    int blocks_to_fall = 0;

    if (game->fall_timer > 1.0f) {
        blocks_to_fall = (int)game->fall_timer;
        game->fall_timer -= (float)blocks_to_fall;

        int hard_drop_distance = game->hard_drop_distance;
        // dbglog(DBG_INFO, "Current hard drop distance: %d\n", hard_drop_distance);
        if (blocks_to_fall >= hard_drop_distance) {
            blocks_to_fall = hard_drop_distance;
        }
        if (blocks_to_fall == 0) {
            commit_active_tetro(game);
            game->active_tetro.set=1;
        }
        else {
            move_tetro_like_this(&game->active_tetro, 0, blocks_to_fall);
        }
        game->hard_drop_distance = find_hard_drop_distance(game);
    }
}