
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <kos.h>
#include <kos/dbglog.h>

// Parallax is used for text rendering
#include <plx/font.h>
#include <plx/sprite.h>
#include <plx/list.h>
#include <plx/dr.h>

#include <types.h>
#include <main.h>
#include <constants.h>
#include <debug.h>
#include <input.h>
#include <tetromino.h>
#include <render.h>

plx_font_t * fnt;
plx_fcxt_t * fnt_cxt;
point_t w;

int paused = 0;
int pause_button_released=1;

extern uint8 romdisk[];
KOS_INIT_FLAGS(INIT_DEFAULT);
KOS_INIT_ROMDISK(romdisk);

void init(){
    pvr_init_defaults();

    // pvr_set_bg_color(201.0f/255.0f, 195.0f/255.0f, 1.0f);

    // maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_LCD);
    // vmu_draw_lcd(vmu, vmu_carl);

    fnt = plx_font_load("/rd/typewriter.txf");
    fnt_cxt = plx_fcxt_create(fnt, PVR_LIST_TR_POLY);
}

void init_game_instance(GameInstance* game){
    paused = 0;
    memset(game->hold_field, BLOCK_NONE, sizeof(game->hold_field));
    memcpy(game->field, field_backup, 288*sizeof(int));
    game->held_tetro = 0;
    game->hold_eligible = 1; // whether we will let the user perform a tetromino hold
    game->has_drawn_new_tetro = 0;
    game->line_clears = 0;
    game->score = 0;
    game->level = 1;
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

int check_valid_state(GameInstance* game){
    // compare the blocks of the active tetro array and its placement
    // on the grid to the playfield to check for overlaps

    int size = game->active_tetro.info->size;

    for (int relative_y = 0; relative_y < size; relative_y++) {
        for (int relative_x = 0; relative_x < size; relative_x++) {
            int block = game->active_tetro.dummy[relative_y][relative_x];

            if (block != BLOCK_NONE) {
                int field_x = game->active_tetro.left_x + relative_x;
                int field_y = game->active_tetro.top_y + relative_y;

                int field_block = game->field[field_y][field_x];

                if (field_block != BLOCK_NONE) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

void clear_line(GameInstance* game, int rownum){
    // Copy every line above the cleared line down 1 row
    for(int row=rownum; row>=4; row--){
        for(int cell=1; cell<=10; cell++){
            game->field[row][cell] = game->field[row-1][cell];
        }
    }

    // the top visible row is now completely cleared, but we need
    // to restore the border (outside of player's view)
    game->field[TOP_VISIBLE_ROW_INDEX][LEFT_VISIBLE_COLUMN_INDEX-1]=1;
    for(int cell=LEFT_VISIBLE_COLUMN_INDEX; cell<=RIGHT_VISIBLE_COLUMN_INDEX; cell++){
        game->field[TOP_VISIBLE_ROW_INDEX][cell]=0;
    }
    game->field[TOP_VISIBLE_ROW_INDEX][RIGHT_VISIBLE_COLUMN_INDEX+1]=1;
}

void check_lines(GameInstance* game){
    int new_line_clears=0;

    int found_empty_row=0;
    int empty_tiles=0;

    // start from bottom to top
    int row=BOTTOM_VISIBLE_ROW_INDEX;
    while(row>=TOP_VISIBLE_ROW_INDEX && !found_empty_row){
        for(int cell=LEFT_VISIBLE_COLUMN_INDEX; cell<=RIGHT_VISIBLE_COLUMN_INDEX; cell++){
            if(!game->field[row][cell]){
                empty_tiles += 1;
            }
        }
        if(empty_tiles==0){
            clear_line(game, row);
            game->line_clears++;
            new_line_clears++;
        }
        // if the whole row is empty, no point in checking any lines above this
        else if(empty_tiles==10){
            found_empty_row=1;
        }
        empty_tiles=0;
        row--;
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
        }
    }
}

void shuffle_bag(GameInstance* game) {
    for (int i = 6; i > 0; i--) {
        int j = rand() % (i + 1); // pick random index from 0 to i
        TetrominoType temp = game->bag[i];
        game->bag[i] = game->bag[j];
        game->bag[j] = temp;
    }
}

void update_pause(GameInstance* game) {
    if (game->input.button_start.just_pressed) {
        paused = !paused;
    }
}

void advance_game_logic(GameInstance* game){

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
            if(game->active_tetro.set==1 || game->first_run==1){
                check_lines(game);
                generate_new_tetro(game);
                game->hold_eligible=1;
                game->first_run=0;
            }
            process_tetro_fall(game);
        }
    }
}

int main(){
    srand(time(NULL));

    init();
    GameInstance game;
    init_game_instance(&game);

    while(1){
        update_inputs(&game.input, 0);
        advance_game_logic(&game);
        draw_frame_gameplay(&game);
    }

    // maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_LCD);
    // vmu_draw_lcd(vmu, vmu_clear);
    pvr_shutdown();
}
