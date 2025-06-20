#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "types.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FIELD_HEIGHT 400 // 20 blocks x 20 pixels each
#define FIELD_WIDTH 200  // 10 blocks x 20 pixels each

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

// RGBA colors
extern const ColorRgba RGBA_RED;
extern const ColorRgba RGBA_ORANGE;
extern const ColorRgba RGBA_YELLOW;
extern const ColorRgba RGBA_GREEN;
extern const ColorRgba RGBA_CYAN;
extern const ColorRgba RGBA_BLUE;
extern const ColorRgba RGBA_PURPLE;
extern const ColorRgba RGBA_WHITE;
extern const ColorRgba RGBA_BLACK;

// Default field layout
extern const BlockColor field_backup[24][12];

#endif // CONSTANTS_H
