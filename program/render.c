#include <kos.h>
#include <plx/font.h>
#include "types.h"
#include "render.h"
#include "constants.h"
#include "colors.h"

extern int paused;
extern plx_font_t * fnt;
extern plx_fcxt_t * fnt_cxt;
extern point_t w;

void draw_triangle(float x1, float y1,
                   float x2, float y2,
                   float x3, float y3,
                   uint32_t color,
                   int pvr_list_type, float z)
                   {

    // POINTS SUBMITTED MUST BE IN COUNTER-CLOCKWISE ORDER

    pvr_poly_hdr_t hdr;
    pvr_poly_cxt_t cxt;
    pvr_vertex_t vert;

    pvr_poly_cxt_col(&cxt, pvr_list_type);

    if (pvr_list_type == PVR_LIST_TR_POLY) {
        cxt.gen.alpha = PVR_ALPHA_ENABLE;
    }
    cxt.gen.culling = PVR_CULLING_NONE; // ?
    pvr_poly_compile(&hdr, &cxt);
    pvr_prim(&hdr, sizeof(hdr));

    vert.flags = PVR_CMD_VERTEX;
    vert.x = x1;
    vert.y = y1;
    vert.z = z;
    vert.u = 0;
    vert.v = 0;
    vert.argb = color;
    // vert.argb = PVR_PACK_COLOR(argb.a/255.0f, argb.r/255.0f, argb.g/255.0f, argb.b/255.0f);
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

void draw_square(float left, float right, float top, float bottom, uint32_t color, int pvr_list_type, float z) {
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
    vert.argb = color;
    // vert.argb = PVR_PACK_COLOR(argb.a/255.0f, argb.r/255.0f, argb.g/255.0f, argb.b/255.0f);
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
void draw_square_centered_on(float center_x, float center_y, float width, float height, uint32_t color, int pvr_list_type, float z) {
    float left = center_x - (width/2);
    float right = center_x + (width/2);
    float top = center_y - (height/2);
    float bottom = center_y + (height/2);
    draw_square(left, right, top, bottom, color, pvr_list_type, z);
}

void draw_vert_line(float x, float top, float bottom, uint32_t color, int pvr_list_type, float z) {
    draw_square(x, x+1, top, bottom, color, pvr_list_type, z);
}

void draw_horiz_line(float left, float right, float y, uint32_t color, int pvr_list_type, float z) {
    draw_square(left, right, y, y+1, color, pvr_list_type, z);
}

void draw_playfield_grid(GameInstance* game) { // TRANSL
    // draw playfield grid (edges and lines)

    // z = 0.1
    draw_horiz_line(field_left, field_right, field_top, RGB(0xFFFFFF), PVR_LIST_TR_POLY, Z_GRID_BORDER);
    draw_horiz_line(field_left, field_right, field_bottom, RGB(0xFFFFFF), PVR_LIST_TR_POLY, Z_GRID_BORDER);
    draw_vert_line(field_left, field_top, field_bottom, RGB(0xFFFFFF), PVR_LIST_TR_POLY, Z_GRID_BORDER);
    draw_vert_line(field_right, field_top, field_bottom, RGB(0xFFFFFF), PVR_LIST_TR_POLY, Z_GRID_BORDER);

    for(int i = BLOCK_WIDTH_PIXELS; i < FIELD_HEIGHT_PIXELS; i += BLOCK_WIDTH_PIXELS){
        // half opacity black
        // draw_horiz_line(field_left, field_right, field_top + i, (ColorRgba){0, 0, 0, 160}, PVR_LIST_TR_POLY, Z_GRID);
        draw_horiz_line(field_left, field_right, field_top + i, RGBA(0x000000, 160), PVR_LIST_TR_POLY, Z_GRID);
    }

    for(int j = BLOCK_WIDTH_PIXELS; j < FIELD_WIDTH_PIXELS; j += BLOCK_WIDTH_PIXELS){
        draw_vert_line(field_left + j, field_top, field_bottom, RGBA(0x000000, 160), PVR_LIST_TR_POLY, Z_GRID);
    }
}



void draw_field_blocks(GameInstance* game){
    // Needs to be called in the opaque list

    float block_x, block_y;

    // Draw fixed (committed) block
    for(int row = TOP_VISIBLE_ROW_INDEX; row <= BOTTOM_VISIBLE_ROW_INDEX; row++){
        for(int col = LEFT_VISIBLE_COLUMN_INDEX; col <= RIGHT_VISIBLE_COLUMN_INDEX; col++){
            if (game->field[row][col]) {
                block_x = field_left + (BLOCK_WIDTH_PIXELS * (col - LEFT_VISIBLE_COLUMN_INDEX)) + (BLOCK_WIDTH_PIXELS/2);
                block_y = field_top + (BLOCK_WIDTH_PIXELS * (row - TOP_VISIBLE_ROW_INDEX)) + (BLOCK_WIDTH_PIXELS/2);
                draw_block(block_x, block_y, &tetromino_colors[game->field[row][col]]);
                // draw_square_centered_on(block_x, block_y, BLOCK_WIDTH_PIXELS, BLOCK_WIDTH_PIXELS, get_argb_from_blockcolor(game->field[row][col]), PVR_LIST_OP_POLY, Z_BLOCKS);
            }
        }
    }
    int size = game->active_tetro.info->size;

    // draw active tetro from dummy
    game->ghost_count = 0;
    for (int rel_y = 0; rel_y < size; rel_y++) {
        for (int rel_x = 0; rel_x < size; rel_x++) {
            if (game->active_tetro.dummy[rel_y][rel_x] != BLOCK_NONE) {
                int abs_x = game->active_tetro.left_x + rel_x;
                int abs_y = game->active_tetro.top_y + rel_y;

                // only draw visible part of the field
                if (abs_y >= TOP_VISIBLE_ROW_INDEX && abs_y <= BOTTOM_VISIBLE_ROW_INDEX && abs_x >= LEFT_VISIBLE_COLUMN_INDEX && abs_x <= RIGHT_VISIBLE_COLUMN_INDEX) {
                    block_x = field_left + (BLOCK_WIDTH_PIXELS * (abs_x - LEFT_VISIBLE_COLUMN_INDEX)) + (BLOCK_WIDTH_PIXELS/2);
                    block_y = field_top + (BLOCK_WIDTH_PIXELS * (abs_y - TOP_VISIBLE_ROW_INDEX)) + (BLOCK_WIDTH_PIXELS/2);
                    draw_block(block_x, block_y, &tetromino_colors[game->active_tetro.info->color]);
                    // draw_square_centered_on(block_x, block_y, BLOCK_WIDTH_PIXELS, BLOCK_WIDTH_PIXELS, get_argb_from_blockcolor(game->active_tetro.info->color), PVR_LIST_OP_POLY, Z_BLOCKS);
                }
            }
            
            // ghost tetromino
            if (game->ghost_tetro.dummy[rel_y][rel_x] != BLOCK_NONE) {
                int abs_x = game->ghost_tetro.left_x + rel_x;
                int abs_y = game->ghost_tetro.top_y + rel_y;
                if (abs_y >= TOP_VISIBLE_ROW_INDEX && abs_y <= BOTTOM_VISIBLE_ROW_INDEX && abs_x >= LEFT_VISIBLE_COLUMN_INDEX && abs_x <= RIGHT_VISIBLE_COLUMN_INDEX) {
                    block_x = field_left + (BLOCK_WIDTH_PIXELS * (abs_x - LEFT_VISIBLE_COLUMN_INDEX)) + (BLOCK_WIDTH_PIXELS/2);
                    block_y = field_top + (BLOCK_WIDTH_PIXELS * (abs_y - TOP_VISIBLE_ROW_INDEX)) + (BLOCK_WIDTH_PIXELS/2);
                    game->ghost_tiles[game->ghost_count++] = (Point){block_x, block_y};
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
                block_x= hold_left + (BLOCK_WIDTH_PIXELS*(col-1)) + (BLOCK_WIDTH_PIXELS/2);
                block_y = hold_top + (BLOCK_WIDTH_PIXELS*(row-1)) + (BLOCK_WIDTH_PIXELS/2);
                draw_block(block_x, block_y, &tetromino_colors[held_tetro_info->color]);
                // draw_square_centered_on(block_x, block_y, BLOCK_WIDTH_PIXELS, BLOCK_WIDTH_PIXELS, get_argb_from_blockcolor(held_tetro_info->color), PVR_LIST_OP_POLY, Z_BLOCKS);
            }
        }
    }
}

void draw_block(float center_x, float center_y, const BlockColorSet *color_set) {
    float left = center_x - (BLOCK_WIDTH_PIXELS/2);
    float right = center_x + (BLOCK_WIDTH_PIXELS/2);
    float top = center_y - (BLOCK_WIDTH_PIXELS/2);
    float bottom = center_y + (BLOCK_WIDTH_PIXELS/2);

    // dbglog(DBG_INFO, "Block bounds: left=%.2f, right=%.2f, top=%.2f, bottom=%.2f\n", left, right, top, bottom);

    // draw_triangle(left, top, left, bottom, right, top, (ColorRgba){255,255,255,255}, PVR_LIST_OP_POLY, 3.7f);
    draw_triangle(left, top, left, bottom, right, top, color_set->highlight, PVR_LIST_OP_POLY, 3.7f);

    // draw_triangle(left, bottom, right, bottom, right, top, (ColorRgba){0,0,0,255}, PVR_LIST_OP_POLY, 3.7f);
    draw_triangle(left, bottom, right, bottom, right, top, color_set->shadow, PVR_LIST_OP_POLY, 3.7f);

    // draw_triangle(right, top, left, top, left, bottom, (ColorRgba){255, 255, 255, 255}, PVR_LIST_OP_POLY, Z_BLOCKS-1.0f);
    // draw_triangle(right, top, left, bottom, right, bottom, (ColorRgba){0, 0, 0, 255}, PVR_LIST_OP_POLY, Z_BLOCKS-1.0f);
    draw_square_centered_on(center_x, center_y, (float)BLOCK_WIDTH_PIXELS*0.6f, (float)BLOCK_WIDTH_PIXELS*0.6f, color_set->base, PVR_LIST_OP_POLY, Z_BLOCKS);
}

void draw_text(float x, float y, char * text){
    w.x = x;
    w.y = y;
    w.z = Z_TEXT;

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
        // draw_square_centered_on(game->ghost_tiles[i].x, game->ghost_tiles[i].y, BLOCK_WIDTH_PIXELS, BLOCK_WIDTH_PIXELS, (ColorRgba){255, 255, 255, 128}, PVR_LIST_TR_POLY, Z_GHOST);
        draw_square_centered_on(game->ghost_tiles[i].x, game->ghost_tiles[i].y, BLOCK_WIDTH_PIXELS, BLOCK_WIDTH_PIXELS, RGBA(0xFFFFFF, 128), PVR_LIST_TR_POLY, Z_GHOST);
    }
}

void draw_frame_gameplay(GameInstance* game){
    pvr_wait_ready();
    pvr_scene_begin();

    pvr_list_begin(PVR_LIST_OP_POLY);
    draw_square(0, 640, 0, 480, RGB(0xC9C3FF), PVR_LIST_OP_POLY, Z_BG); //background
    draw_opaque_polygons(game);
    pvr_list_finish();

    pvr_list_begin(PVR_LIST_TR_POLY);
    draw_translucent_polygons(game);
    pvr_list_finish();

    pvr_scene_finish();
}

// uint32_t get_argb_from_blockcolor(BlockColor color){
//     switch(color){
//         case BLOCK_RED:
//             return COLOR_RED;
//         case BLOCK_ORANGE:
//             return COLOR_ORANGE;
//         case BLOCK_YELLOW:
//             return COLOR_YELLOW;
//         case BLOCK_GREEN:
//             return COLOR_GREEN;
//         case BLOCK_CYAN:
//             return COLOR_CYAN;
//         case BLOCK_BLUE:
//             return COLOR_BLUE;
//         case BLOCK_PURPLE:
//             return COLOR_PURPLE;
//         default:
//             return COLOR_WHITE;
//     }
// }
