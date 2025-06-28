#ifndef RENDER_H
#define RENDER_H

#include "types.h"

void draw_square(float left, float right, float top, float bottom, uint32_t color, int pvr_list_type, float z);
void draw_square_centered_on(float center_x, float center_y, float width, float height, uint32_t color, int pvr_list_type, float z);
void draw_vert_line(float x, float top, float bottom, uint32_t color, int pvr_list_type, float z);
void draw_horiz_line(float left, float right, float y, uint32_t color, int pvr_list_type, float z);
void draw_playfield_grid(GameInstance* game);
void draw_field_blocks(GameInstance* game);
void draw_hold(GameInstance* game);
void draw_block(float center_x, float center_y, const BlockColorSet *color_set);
void draw_text(float x, float y, char * text);
void draw_hud(GameInstance* game);
void draw_opaque_polygons(GameInstance* game);
void draw_translucent_polygons(GameInstance* game);
void draw_frame_gameplay(GameInstance* game);
// uint32_t get_argb_from_blockcolor(BlockColor color);

#endif // RENDER_H
