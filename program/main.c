//https://dreamcast.wiki/PowerVR_Introduction

// resolution is 640w x 480h

// FOR FONTS: REFER TO:
// C:\Data\kos-examples\parallax\font\font.c


// RUN ON DREAMCAST:
// (after running 'make' and attaching bus 2-2 to the WSL instance)
// sudo /opt/toolchains/dc/bin/dc-tool-ser -x program.elf -t /dev/ttyUSB0 -c /mnt/c/Data/Projects/kallistios-tetris/program/romdisk
//      ^ path to dc tool program             ^ your elf     ^ device        ^ path to your romdisk directory

#include <kos.h>
#include <kos/dbglog.h>

//#include <plx/matrix.h>
//#include <plx/prim.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "types.h"
#include "main.h"
#include "constants.h"
#include "debug.h"
#include "input.h"

// font stuff
#include <plx/font.h>
#include <plx/sprite.h>
#include <plx/list.h>
#include <plx/dr.h>

plx_font_t * fnt;
plx_fcxt_t * fnt_cxt;
point_t w;

int paused = 0;
int pause_button_released=1;

extern uint8 romdisk[];
KOS_INIT_FLAGS(INIT_DEFAULT);
KOS_INIT_ROMDISK(romdisk);

void init_game_instance(GameInstance* game){
    paused = 0;
    // memset(game->temp_field, COLOR_NONE, sizeof(game->temp_field));
    memset(game->hold_field, COLOR_NONE, sizeof(game->hold_field));
    memcpy(game->field, field_backup, 288*sizeof(int));
    game->held_tetro = 0;
    game->hold_eligible = 1; // whether we will let the user perform a tetromino hold
    game->has_drawn_new_tetro = 0;
    game->line_clears = 0;
    game->score = 0;
    game->level = 1;
    // game->falltime = 93;
    // game->fall_timer = 93;
    game->fall_timer = 0.0f;
    game->loss = 0;
    game->first_run = 1;
    game->active_tetro.set = 0;
    game->move_timebuffer = 10;
    game->released_y_button=1;
    game->released_x_button=1;
    game->released_up_button=1;
    game->released_ltrig=1;
    game->bag_index=0;

    // fill tetromino bag
    for (int i=1; i<8; i++) {
        game->bag[i-1] = i;
    }
    shuffle_bag(game);
}

ColorRgba get_argb_from_blockcolor(BlockColor color){
    switch(color){
        case COLOR_RED:
            return RGBA_RED;
        case COLOR_ORANGE:
            return RGBA_ORANGE;
        case COLOR_YELLOW:
            return RGBA_YELLOW;
        case COLOR_GREEN:
            return RGBA_GREEN;
        case COLOR_CYAN:
            return RGBA_CYAN;
        case COLOR_BLUE:
            return RGBA_BLUE;
        case COLOR_PURPLE:
            return RGBA_PURPLE;
        default:
            return RGBA_WHITE;
    }
}

void init(){
    pvr_init_defaults();

    pvr_set_bg_color(0.0,1.0,0.3);

    // maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_LCD);
    // vmu_draw_lcd(vmu, vmu_carl);

    plx_font_t * fnt = plx_font_load("/rd/typewriter.txf");

    fnt_cxt = plx_fcxt_create(fnt, PVR_LIST_TR_POLY);

}

void draw_triangle(float x1, float y1,
                   float x2, float y2,
                   float x3, float y3,
                   ColorRgba argb)
                   {
    // POINTS SUBMITTED MUST BE IN CLOCKWISE ORDER
    pvr_poly_hdr_t hdr;
    pvr_poly_cxt_t cxt;
    pvr_vertex_t vert;

    pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    vert.flags = PVR_CMD_VERTEX;
    vert.x = x1;
    vert.y = y1;
    vert.z = 5.0f;
    vert.u = 0;
    vert.v = 0;
    vert.argb = PVR_PACK_COLOR(argb.a/255, argb.r/255, argb.g/255, argb.b/255);
    vert.oargb = 0;
    pvr_prim(&vert, sizeof(vert));

    vert.x = x2;
    vert.y = y2;
    pvr_prim(&vert, sizeof(vert));

    vert.flags = PVR_CMD_VERTEX_EOL; //tells PVR that this will be the final point
    vert.x = x3;
    vert.y = y3;
    pvr_prim(&vert, sizeof(vert));
}

void draw_square(float left, float right, float top, float bottom, ColorRgba argb) {
    pvr_poly_hdr_t hdr;
    pvr_poly_cxt_t cxt;
    pvr_vertex_t vert;

    // x1 = left
    // x2 = right
    // y1 = top
    // y2 = bottom

    if(top>bottom) {
        float swap_y;
        swap_y = top;
        top = bottom;
        bottom = swap_y;
    }

    if(left>right) {
        float swap_x;
        swap_x = left;
        left = right;
        right = swap_x;
    }

    pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
    pvr_poly_compile(&hdr, &cxt);

    pvr_prim(&hdr, sizeof(hdr));
    vert.flags = PVR_CMD_VERTEX;
    // bottom left
    vert.x = left;
    vert.y = bottom;
    vert.z = 5.0f;
    vert.u = 0;
    vert.v = 0;
    vert.argb = PVR_PACK_COLOR(argb.a/255, argb.r/255, argb.g/255, argb.b/255);
    vert.oargb = 0;
    pvr_prim(&vert, sizeof(vert));

    // top left
    vert.y = top;
    pvr_prim(&vert, sizeof(vert));

    // bottom right
    vert.x = right;
    vert.y = bottom;
    pvr_prim(&vert, sizeof(vert));

    // top right
    vert.flags = PVR_CMD_VERTEX_EOL;
    vert.y = top;
    pvr_prim(&vert, sizeof(vert));
}
void draw_square_centered_on(float center_x, float center_y, float width, float height, ColorRgba argb) {
    float left = center_x - (width/2);
    float right = center_x + (width/2);
    float top = center_y - (height/2);
    float bottom = center_y + (height/2);
    draw_square(left, right, top, bottom, argb);
}

void draw_vert_line(float x, float top, float bottom, ColorRgba argb) {
    draw_square(x, x+1, top, bottom, argb);
}

void draw_horiz_line(float left, float right, float y, ColorRgba argb) {
    draw_square(left, right, y, y+1, argb);
}

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
    // game->active_tetro.dummy = {0};
    game->ghost_tetro.dummy = game->active_tetro.dummy;

    // reset the holder to its initial state
    memset(game->active_tetro.dummy, 0, sizeof(game->active_tetro.dummy));
    for (int i=0; i<info->size; i++) {
        for (int j=0; j<info->size; j++) {
            game->active_tetro.dummy[i][j] = info->shape[i][j];
        }
    }

    game->hard_drop_distance = find_hard_drop_distance(game);//, &game->active_tetro);
    update_ghost_piece(game);
}

// void replot_tetro(GameInstance* game, Tetrodata *tetro){
//     // Adds active_tetro to the temp_field matrix, in the position and
//     // orientation specified by the data members in active_tetro
//     // You have to do this before you check the validity of the fields.

//     memset(game->temp_field, COLOR_NONE, sizeof(game->temp_field));

//     // const TetrominoInfo *info = game->active_tetro.info;
//     const TetrominoInfo *info = tetro->info;

//     for (int row=0; row<info->size; row++){
//         for (int cell=0; cell<info->size; cell++){
//             if (game->active_tetro.top_y+row < 24 && game->active_tetro.left_x+cell < 12) { //don't go off the grid          
//                 if (game->active_tetro.dummy[row][cell] > 0) {
//                     game->temp_field[game->active_tetro.top_y + row][game->active_tetro.left_x + cell] = info->color;
//                 }      
//             }
//         }
//     }
// }

void commit_active_tetro(GameInstance* game){
    // The active tetromino gets copied from the active tetromino array to the main field
    // data structure to "set" it.
    // THIS DOES NOT DO CHECKS to validate position! Check it first with check_valid_state()

    // for(int row=0; row<24; row=row+1){
    //     for(int cell=0; cell<12; cell=cell+1){
    //         if(game->temp_field[row][cell]>0){
    //             game->field[row][cell]=game->temp_field[row][cell];
    //         }
    //     }
    // }
    dbglog(DBG_INFO, "commiting tetromino with left x %d, top y %d\n",game->active_tetro.left_x,game->active_tetro.top_y);

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

// int check_valid_state(GameInstance* game){
//     // Checks for overlapping tiles/blocks between the temp_field (the active tetromino) and
//     // the field (all other tetrominos and the edges of the screen).
//     // Returns 0 (false) if an overlap is found (state is invalid).
//     // Returns 1 (true) if an overlap is NOT found (state is valid)

//     //dbglog(DBG_INFO, "in check_valid_state()");

//     int size =  game->active_tetro.info->size;
    
//     for (int relative_y = 0; relative_y < size; relative_y++) {
//         for (int relative_x = 0; relative_x < size; relative_x++) {
//             if (game->active_tetro.dummy[relative_y][relative_x] != COLOR_NONE) {

//                 // convert relative tetro array coordinates to absolute field coordinates
//                 int field_x = game->active_tetro.left_x + relative_x;
//                 int field_y = game->active_tetro.top_y + relative_y;

//                 // Check bounds
//                 if (field_x < 0 || field_x >= FIELD_WIDTH ||
//                     field_y < 0 || field_y >= FIELD_HEIGHT) {
//                     return 0;
//                 }

//                 // Check for collision with existing blocks
//                 if (game->field[field_y][field_x] != COLOR_NONE) {
//                     return 0;
//                 }
//             }
//         }
//     }

//     // for(int row=0; row<24; row++){
//     //     for(int cell=0; cell<12; cell++){
//     //         if(game->field[row][cell]!=0 && game->temp_field[row][cell]!=0){
//     //             return 0;
//     //         }
//     //     }
//     // }
//     return 1;
// }
int check_valid_state(GameInstance* game){
    //dbglog(DBG_INFO, "in check_valid_state()\n");

    int size = game->active_tetro.info->size;
    //dbglog(DBG_INFO, "Tetromino size: %d\n", size);
    //dbglog(DBG_INFO, "Active tetro position: (%d, %d)\n", game->active_tetro.left_x, game->active_tetro.top_y);

    for (int relative_y = 0; relative_y < size; relative_y++) {
        for (int relative_x = 0; relative_x < size; relative_x++) {
            int block = game->active_tetro.dummy[relative_y][relative_x];
            //dbglog(DBG_INFO, "Checking dummy[%d][%d] = %d\n", relative_y, relative_x, block);

            if (block != COLOR_NONE) {
                int field_x = game->active_tetro.left_x + relative_x;
                int field_y = game->active_tetro.top_y + relative_y;
                //dbglog(DBG_INFO, "--> Block present. Field coords: (%d, %d)\n", field_x, field_y);

                // Check out-of-bounds
                if (field_x < 0 || field_x >= FIELD_WIDTH || field_y < 0 || field_y >= FIELD_HEIGHT) {
                    //dbglog(DBG_ERROR, "!! Out of bounds at (%d, %d). Returning INVALID.\n", field_x, field_y);
                    return 0;
                }

                int field_block = game->field[field_y][field_x];
                //dbglog(DBG_INFO, "--> Field[%d][%d] = %d\n", field_y, field_x, field_block);

                if (field_block != COLOR_NONE) {
                    //dbglog(DBG_ERROR, "!! Collision detected at (%d, %d). Returning INVALID.\n", field_y, field_x);
                    return 0;
                }
            }
        }
    }

    //dbglog(DBG_INFO, "No collisions or bounds issues. Returning VALID.\n");
    return 1;
}


void tetro_left(GameInstance* game){
    game->active_tetro.left_x -= 1;
    // replot_active_tetro(game);
    if(!check_valid_state(game)){
        game->active_tetro.left_x += 1; //undo it
        // replot_active_tetro(game);
    } else {
        game->hard_drop_distance = find_hard_drop_distance(game);//, game->active_tetro);
        update_ghost_piece(game);
    }
}

void tetro_right(GameInstance* game){
    game->active_tetro.left_x += 1;
    // replot_active_tetro(game);
    if(!check_valid_state(game)){
        game->active_tetro.left_x -= 1; //undo it
        // replot_active_tetro(game);
    } else {
        game->hard_drop_distance = find_hard_drop_distance(game);//, game->active_tetro);
        update_ghost_piece(game);
    }
}

void update_ghost_piece(GameInstance* game){
    game->ghost_tetro.top_y = game->active_tetro.top_y + game->hard_drop_distance;
    game->ghost_tetro.left_x = game->active_tetro.left_x;
    dbglog(DBG_INFO, "ghost tetro top y: %d, hard drop distance: %d\n", game->ghost_tetro.top_y, game->hard_drop_distance);
}

void tetro_fall(GameInstance* game, int award_score, int commit){
    // one block at a time
    dbglog(DBG_INFO, "tetro_fall: old tetro top y: %d\n", game->active_tetro.top_y);
    game->active_tetro.top_y += 1;
    dbglog(DBG_INFO, "tetro_fall: new tetro top y: %d\n", game->active_tetro.top_y);

    // replot_active_tetro(game);
    if(!check_valid_state(game)){
        dbglog(DBG_INFO, "%d is invalid\n", game->active_tetro.top_y);
        game->active_tetro.top_y -= 1; //undo it
        // replot_active_tetro(game);
        if (commit) {
            commit_active_tetro(game);
            game->active_tetro.set=1;
        }
    }
    if(award_score){
        game->score+=1;
    }
}

void hard_drop(GameInstance* game) {
    game->active_tetro.top_y += game->hard_drop_distance;
    commit_active_tetro(game);
    game->active_tetro.set=1;
}

// int find_hard_drop_distance(GameInstance* game){
//     // Falls straight down until the tetromino gets set
//     int orig_y = game->active_tetro.top_y;

//     int valid = 1;
//     int blocks_fallen=0;
//     // while(!game->active_tetro.set){
//     while(valid){
//         tetro_fall(game, 0, 0); // 0, 0 = don't aware score, don't commit when invalid pos is found
//         valid = check_valid_state(game);
//         if (valid) {
//             blocks_fallen++;
//         }
//     }
//     // game->score += blocks_fallen * 2;
//     // reset
//     game->active_tetro.top_y = orig_y;
//     return blocks_fallen;
//     // replot_active_tetro()
// }
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

// https://stackoverflow.com/questions/27288694/transpose-of-a-matrix-2d-array

void swap(int* arg1, int* arg2)
// Swap the two values in memory (in place)
// Used in tetromino rotation
{
    int buffer = *arg1;
    *arg1 = *arg2;
    *arg2 = buffer;
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
    // replot_active_tetro(game);

    if (check_valid_state(game)) {
        update_orientation_cw(&game->active_tetro);
        // //dbglog(DBG_INFO, "%c tetromino rotated cw with no kicks\n", game->active_tetro.info->symbol);
        game->hard_drop_distance = find_hard_drop_distance(game);//, &game->active_tetro);
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

        // replot_active_tetro(game);

        if (check_valid_state(game)) {
            update_orientation_cw(&game->active_tetro);
            game->hard_drop_distance = find_hard_drop_distance(game);//, &game->active_tetro);
            update_ghost_piece(game);
            // //dbglog(DBG_INFO, "%c tetromino rotated on test %d (%d, %d)\n", game->active_tetro.info->symbol, test_index+2, info->kicks_cw[rotation_type_index][test_index][0], info->kicks_cw[rotation_type_index][test_index][1]);
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

    // replot_active_tetro(game);
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

    // replot_active_tetro(game);

    if (check_valid_state(game)) {
        update_orientation_ccw(&game->active_tetro);
        game->hard_drop_distance = find_hard_drop_distance(game);//. &game->active_tetro);
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

        // replot_active_tetro(game);

        if (check_valid_state(game)) {
            update_orientation_ccw(&game->active_tetro);
            game->hard_drop_distance = find_hard_drop_distance(game);//, &game->active_tetro);
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

    // replot_active_tetro(game);
}

void hold_tetromino(GameInstance* game){

    const TetrominoInfo *tetromino_to_hold = game->active_tetro.info;
    
    if(game->held_tetro){ //if there's currently a tetromino already in the hold
    // swap it
        init_new_tetro(game, game->held_tetro->type);
        // replot_active_tetro(game);

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
        // dbg_print_field(game);
        hard_drop(game);
        game->move_timebuffer=10;
        return 0;
    }

    if(game->move_timebuffer<=0){
        // dbg_print_field(game);
        if(game->input.dpad_down.pressed){
            // softdrop
            tetro_fall(game, 1, 1);
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

    //dbglog(DBG_INFO, "in genrate_new_tetro: calling init_new_tetro()\n");

    init_new_tetro(game, random_id);
    //dbglog(DBG_INFO, "init_new_tetro() done");
    // replot_active_tetro(game);

    if(!check_valid_state(game)){
        game->loss = 1;
    }
    //dbglog(DBG_INFO, "done with check_valid_state()");
}

// void draw_field(GameInstance* game){
//     // One block is 20 pixels x 20 pixels
//     // field is 20 blocks * 20 pixels tall = 400 pixels
//     // and 10 blocks * 20 pixels wide = 200 pixels

//     //draw edges
//     draw_horiz_line(field_left, field_right, field_top, RGBA_WHITE);
//     draw_horiz_line(field_left, field_right, field_bottom, RGBA_WHITE);
//     draw_vert_line(field_left, field_top, field_bottom, RGBA_WHITE);
//     draw_vert_line(field_right, field_top, field_bottom, RGBA_WHITE);

//     //draw each row
//     for(int i=20; i<FIELD_HEIGHT; i=i+20){
//         draw_horiz_line(field_left, field_right, field_top+i, RGBA_BLACK);
//     }

//     //draw each column
//     for(int j=20; j<FIELD_WIDTH; j=j+20){
//         draw_vert_line(field_left+j, field_top, field_bottom, RGBA_BLACK);
//     }

//     float block_x;
//     float block_y;
//     //now draw the blocks
//     for(int row=3; row<23; row=row+1){
//         for(int col=1;col<11; col=col+1){
//             if (game->field[row][col]){
//                 block_x = field_left + (20*(col-1)) + 10;
//                 block_y = field_top + (20*(row-3)) + 10;
//                 draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_blockcolor(game->field[row][col]));
//             }
//             if (game->temp_field[row][col]){
//                 block_x = field_left + (20*(col-1)) + 10;
//                 block_y = field_top + (20*(row-3)) + 10;
//                 draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_blockcolor(game->temp_field[row][col]));
//             }
//         }
//     }
// }
void draw_field(GameInstance* game){
    // draw playfield grid (edges and lines)
    draw_horiz_line(field_left, field_right, field_top, RGBA_WHITE);
    draw_horiz_line(field_left, field_right, field_bottom, RGBA_WHITE);
    draw_vert_line(field_left, field_top, field_bottom, RGBA_WHITE);
    draw_vert_line(field_right, field_top, field_bottom, RGBA_WHITE);

    for(int i = 20; i < FIELD_HEIGHT; i += 20){
        draw_horiz_line(field_left, field_right, field_top + i, RGBA_BLACK);
    }

    for(int j = 20; j < FIELD_WIDTH; j += 20){
        draw_vert_line(field_left + j, field_top, field_bottom, RGBA_BLACK);
    }

    float block_x, block_y;

    // Draw fixed (committed) blocks
    for(int row = 3; row < 23; row++){
        for(int col = 1; col < 11; col++){
            if (game->field[row][col]) {
                block_x = field_left + (20 * (col - 1)) + 10;
                block_y = field_top + (20 * (row - 3)) + 10;
                draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_blockcolor(game->field[row][col]));
            }
        }
    }

    // draw active tetro from dummy
    int size = game->active_tetro.info->size;
    for (int rel_y = 0; rel_y < size; rel_y++) {
        for (int rel_x = 0; rel_x < size; rel_x++) {
            if (game->active_tetro.dummy[rel_y][rel_x] != COLOR_NONE) {
                int abs_x = game->active_tetro.left_x + rel_x;
                int abs_y = game->active_tetro.top_y + rel_y;

                // only draw visible part of the field
                if (abs_y >= 3 && abs_y < 23 && abs_x >= 1 && abs_x < 11) {
                    block_x = field_left + (20 * (abs_x - 1)) + 10;
                    block_y = field_top + (20 * (abs_y - 3)) + 10;
                    draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_blockcolor(game->active_tetro.info->color));
                }
            }
        }
    }

    // draw ghost tetromino
    for (int rel_y = 0; rel_y < size; rel_y++) {
        for (int rel_x = 0; rel_x < size; rel_x++) {
            if (game->ghost_tetro.dummy[rel_y][rel_x] != COLOR_NONE) {
                int abs_x = game->ghost_tetro.left_x + rel_x;
                int abs_y = game->ghost_tetro.top_y + rel_y;
                // dbglog(DBG_INFO, "ghost abs x: %d, ghost abs y: %d\n", abs_x, abs_y);
                if (abs_y >= 3 && abs_y < 23 && abs_x >= 1 && abs_x < 11) {
                    block_x = field_left + (20 * (abs_x - 1)) + 10;
                    block_y = field_top + (20 * (abs_y - 3)) + 10;
                    
                    draw_square_centered_on(block_x, block_y, 20, 20, RGBA_WHITE);
                }
            }
        }
    }

}

void clear_line(GameInstance* game, int rownum){
    // Copy every line above the cleared line down 1 row
    for(int row=rownum; row>=4; row--){
        for(int cell=1; cell<=10; cell++){
            game->field[row][cell] = game->field[row-1][cell];
        }
    }

    game->field[3][0]=1;
    for(int cell=1; cell<=10; cell++){
        game->field[3][cell]=0;
    }
    game->field[3][11]=1;
}

void check_lines(GameInstance* game){
    int found_empty_tile=0;
    
    int new_line_clears=0;

    for(int row=3; row<=22; row++){
        int cell=1;
        while(cell<=10 && !found_empty_tile){
            if(!game->field[row][cell]){
                found_empty_tile=1;
            }
            cell++;
        }
        if(!found_empty_tile){
            clear_line(game, row);
            game->line_clears++;
            new_line_clears++;
        }
        found_empty_tile=0;
    }
    if(new_line_clears==1){
        game->score += game->level * 100;
    }
    else if(new_line_clears==2){
        game->score += game->level * 300;
    }
    else if(new_line_clears==3){
        game->score += game->level * 500;
    }
    else if(new_line_clears==4){
        game->score += game->level * 800;
    }

    if(game->level < 15){
        int new_level = ((game->line_clears-(game->line_clears%10))/10)+1;
        if(new_level>game->level){
            game->level=new_level;
            // game->falltime= 93 - (new_level*6);
        }
    }
    // FALL TIME CALCULATION:
    // Level 1 - falls every 87 frames
    // Level 15 - falls every 2 frames
    // Equation:  y = -6x + 93
    // falltime = -6*level + 93
}

void shuffle_bag(GameInstance* game) {
    for (int i = 6; i > 0; i--) {
        int j = rand() % (i + 1); // pick random index from 0 to i
        TetrominoType temp = game->bag[i];
        game->bag[i] = game->bag[j];
        game->bag[j] = temp;
    }
}

void draw_hold(GameInstance* game){
    if(!game->held_tetro){
        return;
    }

    int hold_left = 50;
    int hold_top = 50;

    float block_x;
    float block_y;

    const TetrominoInfo *held_tetro_info = game->held_tetro;
    int size = held_tetro_info->size;

    for(int row=0; row<size; row++){
        for(int col=0; col<size; col++){
            if(held_tetro_info->shape[row][col]){
                block_x= hold_left + (20*(col-1)) + 10;
                block_y = hold_top + (20*(row-1)) + 10;
                draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_blockcolor(held_tetro_info->color));
            }
        }
    }
}

// void clone_tetromino(Tetrodata *dst, const Tetrodata *src) {
//     *dst = *src;
// }

// void update_ghost_piece(GameInstance *game)
    

void draw_text(float x, float y, char * text){
    w.x = x;
    w.y = y;
    w.z = 5.0f;

    plx_fcxt_begin(fnt_cxt);
    plx_fcxt_setpos_pnt(fnt_cxt, &w);
    plx_fcxt_draw(fnt_cxt, text);
    plx_fcxt_end(fnt_cxt);
}

void draw_hud(GameInstance* game){

    char score_string[10];
    char lines_string[10];
    char level_string[10];

    draw_text(50,300,"Score");
    sprintf(score_string, "%ld", game->score);
    draw_text(50,340,score_string);

    draw_text(500,200,"Level");
    sprintf(level_string, "%d", game->level);
    draw_text(500,240,level_string);

    draw_text(500,300,"Lines");
    sprintf(lines_string, "%d", game->line_clears);
    draw_text(500,340,lines_string);

    draw_hold(game);
}

void update_pause(GameInstance* game) {
    if (game->input.button_start.just_pressed) {
        paused = !paused;
    }
}

// int find_lowest_filled_cell_in_col(BlockColor *column, int height){
//     for (int i=height-1; i>=0; i--){
//         if (column[i]>0) {
//             return i;
//         }
//     }
//     return -1; // none found
// }

// int find_highest_filled_cell_in_col(BlockColor *column, int height){
//     for (int i=0; i<height; i++){
//         if (column[i]>0) {
//             return i;
//         }
//     }
//     return -1;
// }

// // void get_column(BlockColor *matrix, BlockColor *col_buffer, int col_index, int height) {
// //     for (int row=0; row<height; row++) {
// //         col_buffer[row] = matrix[row][col_index];
// //     }
// // }
// void get_column(BlockColor *matrix, BlockColor *col_buffer, int col_index, int col_height, int row_width) {
//     for (int row_index = 0; row_index < col_height; row_index++) {
//         col_buffer[row_index] = matrix[row_index*row_width + col_index];
//     }
// }

// int array_min(int *arr, int size) {
//     if (size <= 0) return -1;

//     int min = arr[0];
//     for (int i = 1; i < size; i++) {
//         if (arr[i] < min) {
//             min = arr[i];
//         }
//     }
//     return min;
// }

// int determine_hard_drop_distance(GameInstance *game) {
//     // TODO: maybe this should be stored in the active_tetro at the start of the frame ??
//     int size = game->active_tetro.info->size;

//     int distances[size];

//     for (int tetro_col_index=0; tetro_col_index<size; tetro_col_index++){
//         BlockColor tetro_col[size];
//         get_column(&game->active_tetro.dummy[0][0], tetro_col, tetro_col_index, size, size);
//         int lowest_tetro_cell = find_lowest_filled_cell_in_col(tetro_col, size);

//         // 'i' is referring to the dummy's indexes which are relative to the tetromino
//         // itself; we need to find where this really exists on the field given the
//         // tetromino's current positioning, in order to compare the right columns
//         int field_x = tetro_col_index + game->active_tetro.left_x;

//         const BlockColor (*trimmed_field)[12] = &game->field[3]; // field, but ignoring the top 3 rows
//         BlockColor field_col[21]; // height of full field (24) minus the 3 we chopped off
//         get_column(&trimmed_field[0][0], field_col, field_x, 21, 12);
//         int highest_field_cell = find_highest_filled_cell_in_col(field_col, 21);

//         if (highest_field_cell==-1){
//             // this should never happen as the bottom row should always be filled and it 
//             // is included in the call, but this is a failsafe
//             highest_field_cell = 21;
//         }

//         distances[tetro_col_index] = highest_field_cell+3 - (lowest_tetro_cell+(game->active_tetro.top_y));
//     }
//     return array_min(distances, size);
// }

// int time_until_lock = 15;

void process_tetro_fall(GameInstance *game) {
    float gravity = gravity_by_level[game->level];
    game->fall_timer += gravity;
    int blocks_to_fall = 0;
    // //dbglog(DBG_INFO, "gravity: %f\nfall timer: %f\n", gravity, game->fall_timer);

    //dbglog(DBG_INFO, "process_tetro_fall: fall timer: %f\n", game->fall_timer);
    //dbglog(DBG_INFO, "process_tetro_fall: hard drop distance: %d\n", game->hard_drop_distance);

    if (game->fall_timer > 1.0f) {
        blocks_to_fall = (int)game->fall_timer;
        game->fall_timer -= (float)blocks_to_fall;

        int hard_drop_distance = game->hard_drop_distance;
        dbglog(DBG_INFO, "Current hard drop distance: %d\n", hard_drop_distance);
        if (blocks_to_fall >= hard_drop_distance) {
            blocks_to_fall = hard_drop_distance;
        }
        if (blocks_to_fall == 0) {
            // if (time_until_lock > 0) {
                // time_until_lock--;
                //dbglog(DBG_INFO, "Time until lock: %d", time_until_lock);
            // }
            // else {
            commit_active_tetro(game);
            game->active_tetro.set=1;
                // time_until_lock = 15;
            // }
        }
        else {
            move_tetro_like_this(&game->active_tetro, 0, blocks_to_fall);
        }
        game->hard_drop_distance = find_hard_drop_distance(game);
        // replot_active_tetro(game);
    }
}

void draw_frame_gameplay(GameInstance* game){

    // pressing start after game is lost = restart game
    if(game->loss) {
        if(game->input.button_start.just_pressed){
            paused=0;
            init_game_instance(game);
        }
    }

    else {
        update_pause(game);
        
        if(!paused){
            process_tetro_movement(game);
            //dbglog(DBG_INFO, "process_tetro_movement() done\n");


            // game->fall_timer -= 1;

            if(game->active_tetro.set==1 || game->first_run==1){
                //dbglog(DBG_INFO, "calling check_lines()\n");
                check_lines(game);
                //dbglog(DBG_INFO, "calling generate_new_tetro()\n");
                generate_new_tetro(game);
                game->hold_eligible=1;
                // game->fall_timer=game->falltime;
                game->first_run=0;
            }
            //dbglog(DBG_INFO, "calling process_tetro_fall()\n");
            process_tetro_fall(game);
            //dbglog(DBG_INFO, "process_tetro_fall() done\n");

            // if(game->fall_timer<=0){
            //     game->fall_timer=game->falltime;
            //     tetro_fall(game, 0);
            // }
        }
    }


    pvr_wait_ready(); // <-- Prevents those ugly flashes!
    pvr_scene_begin();

    pvr_list_begin(PVR_LIST_OP_POLY);
    //opaque drawing here
    pvr_list_finish();

    pvr_list_begin(PVR_LIST_TR_POLY);
    //translucent drawing here
    
    draw_field(game);

    draw_hud(game);

    if(game->loss){
        draw_text(50,200,"You lost!");
        draw_text(50,250,"Press START to reset");
    }

    if (paused){
        draw_text(50, 200, "PAUSED");
    }

    pvr_list_finish();

    pvr_scene_finish();
}

int main(){
    srand(time(NULL));

    int exitProgram = 0;

    //dbglog(DBG_INFO, "Calling init()\n");
    init();

    //dbglog(DBG_INFO, "HELLO WORLD!!!\n");


    GameInstance game;
    //dbglog(DBG_INFO, "Calling init_game_instance()\n");
    init_game_instance(&game);
    //dbglog(DBG_INFO, "init_game_instance() done\n");

    while(!exitProgram){
        update_inputs(&game.input, 0);
        //dbglog(DBG_INFO, "update_inputs() done\n");

        // dbg_print_controller(&game.input);
        // update_logic(&game);
        draw_frame_gameplay(&game);
        //dbglog(DBG_INFO, "draw_frame_gameplay() done\n");
    }

    // maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_LCD);
    // vmu_draw_lcd(vmu, vmu_clear);
    pvr_shutdown();

}
