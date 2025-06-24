#include <kos.h>
#include <plx/font.h>
#include "types.h"
#include "render.h"
#include "constants.h"

extern int paused;
extern plx_font_t * fnt;
extern plx_fcxt_t * fnt_cxt;
extern point_t w;

void draw_square(float left, float right, float top, float bottom, ColorRgba argb, int pvr_list_type, float z) {
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

    pvr_poly_cxt_col(&cxt, pvr_list_type);

    if (pvr_list_type == PVR_LIST_TR_POLY) {
        cxt.gen.alpha = PVR_ALPHA_ENABLE;
    }

    pvr_poly_compile(&hdr, &cxt);

    pvr_prim(&hdr, sizeof(hdr));
    vert.flags = PVR_CMD_VERTEX;
    // bottom left
    vert.x = left;
    vert.y = bottom;
    // vert.z = 5.0f;
    vert.z = z;
    vert.u = 0;
    vert.v = 0;
    vert.argb = PVR_PACK_COLOR(argb.a/255.0f, argb.r/255.0f, argb.g/255.0f, argb.b/255.0f);
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
void draw_square_centered_on(float center_x, float center_y, float width, float height, ColorRgba argb, int pvr_list_type, float z) {
    float left = center_x - (width/2);
    float right = center_x + (width/2);
    float top = center_y - (height/2);
    float bottom = center_y + (height/2);
    draw_square(left, right, top, bottom, argb, pvr_list_type, z);
}

void draw_vert_line(float x, float top, float bottom, ColorRgba argb, int pvr_list_type, float z) {
    draw_square(x, x+1, top, bottom, argb, pvr_list_type, z);
}

void draw_horiz_line(float left, float right, float y, ColorRgba argb, int pvr_list_type, float z) {
    draw_square(left, right, y, y+1, argb, pvr_list_type, z);
}

void draw_playfield_grid(GameInstance* game) { // TRANSL
    // draw playfield grid (edges and lines)

    // z = 0.1
    draw_horiz_line(field_left, field_right, field_top, RGBA_WHITE, PVR_LIST_TR_POLY, 5.0f);
    draw_horiz_line(field_left, field_right, field_bottom, RGBA_WHITE, PVR_LIST_TR_POLY, 5.0f);
    draw_vert_line(field_left, field_top, field_bottom, RGBA_WHITE, PVR_LIST_TR_POLY, 5.0f);
    draw_vert_line(field_right, field_top, field_bottom, RGBA_WHITE, PVR_LIST_TR_POLY, 5.0f);

    // z = 0.2
    // opacity = 50
    const ColorRgba black_half_opac = {
        0, 0, 0, 160
    };
    for(int i = 20; i < FIELD_HEIGHT_PIXELS; i += 20){
        draw_horiz_line(field_left, field_right, field_top + i, black_half_opac, PVR_LIST_TR_POLY, 4.6f);
    }

    for(int j = 20; j < FIELD_WIDTH_PIXELS; j += 20){
        draw_vert_line(field_left + j, field_top, field_bottom, black_half_opac, PVR_LIST_TR_POLY, 4.6f);
    }
}



void draw_field_blocks(GameInstance* game){
    // Needs to be called in the opaque list

    float block_x, block_y;

    // Draw fixed (committed) blocks
    for(int row = 3; row < 23; row++){
        for(int col = 1; col < 11; col++){
            if (game->field[row][col]) {
                block_x = field_left + (20 * (col - 1)) + 10;
                block_y = field_top + (20 * (row - 3)) + 10;
                draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_blockcolor(game->field[row][col]), PVR_LIST_OP_POLY, 4.0f);
            }
        }
    }
    int size = game->active_tetro.info->size;

    // draw active tetro from dummy
    game->ghost_count = 0;
    for (int rel_y = 0; rel_y < size; rel_y++) {
        for (int rel_x = 0; rel_x < size; rel_x++) {
            if (game->active_tetro.dummy[rel_y][rel_x] != COLOR_NONE) {
                int abs_x = game->active_tetro.left_x + rel_x;
                int abs_y = game->active_tetro.top_y + rel_y;

                // only draw visible part of the field
                if (abs_y >= 3 && abs_y < 23 && abs_x >= 1 && abs_x < 11) {
                    block_x = field_left + (20 * (abs_x - 1)) + 10;
                    block_y = field_top + (20 * (abs_y - 3)) + 10;
                    draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_blockcolor(game->active_tetro.info->color), PVR_LIST_OP_POLY, 4.0f);
                }
            }
            
            // ghost tetromino
            if (game->ghost_tetro.dummy[rel_y][rel_x] != COLOR_NONE) {
                int abs_x = game->ghost_tetro.left_x + rel_x;
                int abs_y = game->ghost_tetro.top_y + rel_y;
                if (abs_y >= 3 && abs_y < 23 && abs_x >= 1 && abs_x < 11) {
                    block_x = field_left + (20 * (abs_x - 1)) + 10;
                    block_y = field_top + (20 * (abs_y - 3)) + 10;
                    game->ghost_tiles[game->ghost_count++] = (Point){block_x, block_y};
                    // draw_square_centered_on(block_x, block_y, 20, 20, RGBA_WHITE);
                }
            }
        }
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
                draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_blockcolor(held_tetro_info->color), PVR_LIST_OP_POLY, 4.0f);
            }
        }
    }
}

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
}

void draw_opaque_polygons(GameInstance* game){
    draw_field_blocks(game);
    draw_hold(game);
}


void draw_translucent_polygons(GameInstance* game){
    if(game->loss){
        draw_text(50,200,"You lost!");
        draw_text(50,250,"Press START to reset");
    }
    if (paused){
        draw_text(50, 200, "PAUSED");
    }
    draw_playfield_grid(game);
    draw_hud(game);

    // ghost tiles
    for (int i=0; i<game->ghost_count; i++){
        draw_square_centered_on(game->ghost_tiles[i].x, game->ghost_tiles[i].y, 20, 20, (ColorRgba){255, 255, 255, 128}, PVR_LIST_TR_POLY, 3.5f);
    }
}

void draw_frame_gameplay(GameInstance* game){
    pvr_wait_ready();
    pvr_scene_begin();

    pvr_list_begin(PVR_LIST_OP_POLY);
    draw_square(0, 640, 0, 480, (ColorRgba){201, 195, 255, 255}, PVR_LIST_OP_POLY, 0.1f); //background
    draw_opaque_polygons(game);
    pvr_list_finish();

    pvr_list_begin(PVR_LIST_TR_POLY);
    draw_translucent_polygons(game);
    pvr_list_finish();

    pvr_scene_finish();
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

void draw_background(float r, float g, float b) {
   
}
