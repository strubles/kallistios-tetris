#include "types.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FIELD_HEIGHT 400 // 20 blocks x 20 pixels each
#define FIELD_WIDTH 200 // 10 blocks x 20 pixels each


// Guideline SRS wall kick test relative offsets

// 1st level: the rotation type
// 2nd level: the test number
// 3rd level: x, y offset relative to the last test

// kicks_cw and kicks_ccw are for all tetrominos except I
const int kicks_cw[4][5][2] = {
    //test2    test3    test4    test5    undo
    { {-1, 0}, { 0, 1}, { 1,-3}, {-1, 0}, { 1, 2} }, // 0 -> R
    { { 1, 0}, { 0,-1}, {-1, 3}, { 1, 0}, {-1,-2} }, // R -> 2
    { { 1, 0}, { 0, 1}, {-1,-3}, { 1, 0}, {-1, 2} }, // 2 -> L
    { {-1, 0}, { 0,-1}, { 1, 3}, {-1, 0}, { 1,-2} }  // L -> 0
};
const int kicks_ccw[4][5][2] = {
    { { 1, 0}, { 0, 1}, {-1,-3}, { 1, 0}, {-1, 2} }, // 0 -> L
    { {-1, 0}, { 0,-1}, { 1, 3}, {-1, 0}, { 1,-2} }, // L -> 2
    { {-1, 0}, { 0, 1}, { 1,-3}, {-1, 0}, { 1, 2} }, // 2 -> R
    { { 1, 0}, { 0,-1}, {-1, 3}, { 1, 0}, {-1,-2} }  // R -> 0
};

// kicks_cw_i and kicks_ccw_i apply only to Light Blue/I tetrominos
const int kicks_cw_i[4][5][2] = {
    { {-2, 0}, { 3, 0}, {-3,-1}, { 3, 3}, {-1,-2} }, // 0 -> R
    { {-1, 0}, { 3, 0}, {-3, 2}, { 3,-3}, {-2, 1} }, // R -> 2
    { { 2, 0}, {-3, 0}, { 3, 1}, {-3,-3}, { 1, 2} }, // 2 -> L
    { { 1, 0}, {-3, 0}, { 3,-2}, {-3, 3}, { 2,-1} }  // L -> 0
};
const int kicks_ccw_i[4][5][2] = {
    { {-1, 0}, { 3, 0}, {-3, 2}, { 3,-3}, {-2, 1} }, // 0 -> L
    { {-2, 0}, { 3, 0}, {-3,-1}, { 3, 3}, {-1,-2} }, // R -> 0
    { { 1, 0}, {-3, 0}, { 3,-2}, {-3, 3}, { 2,-1} }, // 2 -> R
    { { 2, 0}, {-3, 0}, { 3, 1}, {-3,-3}, { 1, 2} }  // L -> 2
};

// define tetromino info array constant
const TetrominoInfo tetromino_infos[TETRO_COUNT] = {
    [TETRO_I] = {
        .type = TETRO_I,
        .symbol = 'I',
        .color = COLOR_CYAN,
        .size = 4,
        .shape = {
            { 0, 0, 0, 0 },
            { 1, 1, 1, 1 },
            { 0, 0, 0, 0 },
            { 0, 0, 0, 0 }
        },
        .perform_kicks = 1,
        .kicks_cw = kicks_cw_i,
        .kicks_ccw = kicks_ccw_i,
        .initial_left_x = 4,
        .initial_top_y = 3
    },
    [TETRO_O] = {
        .type = TETRO_O,
        .symbol = 'O',
        .color = COLOR_YELLOW,
        .size = 2,
        .shape = {
            { 1, 1 },
            { 1, 1 }
        },
        .perform_kicks = 0,
        .kicks_cw = kicks_cw, // not used
        .kicks_ccw = kicks_ccw,
        .initial_left_x = 5,
        .initial_top_y = 3
    },
    [TETRO_T] = {
        .type = TETRO_T,
        .symbol = 'T',
        .color = COLOR_PURPLE,
        .size = 3,
        .shape = {
            { 0, 1, 0 },
            { 1, 1, 1 },
            { 0, 0, 0 }
        },
        .perform_kicks = 1,
        .kicks_cw = kicks_cw,
        .kicks_ccw = kicks_ccw,
        .initial_left_x = 4,
        .initial_top_y = 3
    },
    [TETRO_S] = {
        .type = TETRO_S,
        .symbol = 'S',
        .color = COLOR_GREEN,
        .size = 3,
        .shape = {
            { 0, 1, 1 },
            { 1, 1, 0 },
            { 0, 0, 0 }
        },
        .perform_kicks = 1,
        .kicks_cw = kicks_cw,
        .kicks_ccw = kicks_ccw,
        .initial_left_x = 4,
        .initial_top_y = 3
    },
    [TETRO_Z] = {
        .type = TETRO_Z,
        .symbol = 'Z',
        .color = COLOR_RED,
        .size = 3,
        .shape = {
            { 1, 1, 0 },
            { 0, 1, 1 },
            { 0, 0, 0 }
        },
        .perform_kicks = 1,
        .kicks_cw = kicks_cw,
        .kicks_ccw = kicks_ccw,
        .initial_left_x = 4,
        .initial_top_y = 3
    },
    [TETRO_J] = {
        .type = TETRO_J,
        .symbol = 'J',
        .color = COLOR_BLUE,
        .size = 3,
        .shape = {
            { 1, 0, 0 },
            { 1, 1, 1 },
            { 0, 0, 0 }
        },
        .perform_kicks = 1,
        .kicks_cw = kicks_cw,
        .kicks_ccw = kicks_ccw,
        .initial_left_x = 4,
        .initial_top_y = 3
    },
    [TETRO_L] = {
        .type = TETRO_L,
        .symbol = 'L',
        .color = COLOR_ORANGE,
        .size = 3,
        .shape = {
            { 0, 0, 1 },
            { 1, 1, 1 },
            { 0, 0, 0 }
        },
        .perform_kicks = 1,
        .kicks_cw = kicks_cw,
        .kicks_ccw = kicks_ccw,
        .initial_left_x = 4,
        .initial_top_y = 3
    }
};

const int field_left = (SCREEN_WIDTH/2) - (FIELD_WIDTH/2);
const int field_right = (SCREEN_WIDTH/2) + (FIELD_WIDTH/2);
const int field_top = (SCREEN_HEIGHT/2) - (FIELD_HEIGHT/2);
const int field_bottom = (SCREEN_HEIGHT/2) + (FIELD_HEIGHT/2);

const ColorRgba RGBA_RED = {255, 0, 0, 255};
const ColorRgba RGBA_ORANGE = {255, 174, 94, 255};
const ColorRgba RGBA_YELLOW = {255, 255, 0, 255};
const ColorRgba RGBA_GREEN = {0, 255, 0, 255};
const ColorRgba RGBA_CYAN = {0, 255, 255, 255};
const ColorRgba RGBA_BLUE = {0, 0, 255, 255};
const ColorRgba RGBA_PURPLE = {255, 0, 255, 255};
const ColorRgba RGBA_WHITE = {255, 255, 255, 255};
const ColorRgba RGBA_BLACK = {0, 0, 0, 255};

// setting up the default field state
// Only rows 3-22 and columns 1-10 are visible to player

// Border of 1's (cyan blocks) outside of player's view
// enables easy wall/floor collision & loss detection
const BlockColor field_backup[24][12] = {
//    0       1  2  3  4  5  6  7  8  9  10      11
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 0
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 1
    { 1, /**/ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /**/ 1}, // 2
    /**********************************************/
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 3
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 4
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 5
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 6
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 7
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 8
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 9
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 10
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 11
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 12
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 13
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 14
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 15
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 16
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 17
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 18
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 19
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 20
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 21
    { 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 22
    /**********************************************/
    { 1, /**/ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /**/ 1}, // 23

};
