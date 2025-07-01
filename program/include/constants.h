#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <types.h>

#define RGB(rgb)      ((0xFF << 24) | ((rgb) & 0xFFFFFF)) // default opaque
#define RGBA(rgb, a)  (((a) << 24) | ((rgb) & 0xFFFFFF))

#define SCREEN_WIDTH_PIXELS 640
#define SCREEN_HEIGHT_PIXELS 480
#define FIELD_HEIGHT_PIXELS 400 // 20 blocks x 20 pixels each
#define FIELD_WIDTH_PIXELS 200  // 10 blocks x 20 pixels each

#define BLOCK_WIDTH_PIXELS 20

// layering
#define Z_TEXT       5.0f   // front-most
#define Z_GRID_BORDER 5.0f
#define Z_BLOCKS     4.0f
#define Z_GRID       3.7f
#define Z_GHOST      3.5f
#define Z_BG         0.1f   // back-most

// the whole field is never rendered to the screen
// see defintion of field_backup in constants.h for illustration of this
#define TOP_VISIBLE_ROW_INDEX 3
#define BOTTOM_VISIBLE_ROW_INDEX 22
#define LEFT_VISIBLE_COLUMN_INDEX 1
#define RIGHT_VISIBLE_COLUMN_INDEX 10

// Kick table declarations
extern const int kicks_cw[4][5][2];
extern const int kicks_ccw[4][5][2];
extern const int kicks_cw_i[4][5][2];
extern const int kicks_ccw_i[4][5][2];

// Tetromino definitions
extern const TetrominoInfo tetromino_infos[TETRO_COUNT];

// Field bounds in pixels
extern const int field_left;
extern const int field_right;
extern const int field_top;
extern const int field_bottom;

extern const BlockColorSet tetromino_colors[TETRO_COUNT];

// Default field layout
extern const BlockColor field_backup[24][12];

extern const float gravity_by_level[21];

#endif // CONSTANTS_H
