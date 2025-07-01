#include <types.h>
#include <constants.h>

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
        .color = BLOCK_CYAN,
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
        .color = BLOCK_YELLOW,
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
        .color = BLOCK_PURPLE,
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
        .color = BLOCK_GREEN,
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
        .color = BLOCK_RED,
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
        .color = BLOCK_BLUE,
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
        .color = BLOCK_ORANGE,
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

const int field_left = (SCREEN_WIDTH_PIXELS/2) - (FIELD_WIDTH_PIXELS/2);
const int field_right = (SCREEN_WIDTH_PIXELS/2) + (FIELD_WIDTH_PIXELS/2);
const int field_top = (SCREEN_HEIGHT_PIXELS/2) - (FIELD_HEIGHT_PIXELS/2);
const int field_bottom = (SCREEN_HEIGHT_PIXELS/2) + (FIELD_HEIGHT_PIXELS/2);

const BlockColorSet tetromino_colors[TETRO_COUNT] = {
    [BLOCK_RED] = {
        .base       = 0xffdc3545, // #dc3545
        .highlight  = 0xfff67456, // #f67456
        .shadow     = 0xffbe2457  // #be2457
    },
    [BLOCK_ORANGE] = {
        .base       = 0xfffd7e14, // #fd7e14
        .highlight  = 0xffffaa26, // #ffaa26
        .shadow     = 0xfffd4514  // #fd4514
    },
    [BLOCK_YELLOW] = {
        .base       = 0xffffc107, // #ffc107
        .highlight  = 0xfffff200, // #fff200
        .shadow     = 0xffff9307, // #ff9307
    },
    [BLOCK_GREEN] = {
        .base       = 0xff28a745, // #28a745
        .highlight  = 0xff5edb32, // #5edb32
        .shadow     = 0xff177f59  // #177f59
    },
    [BLOCK_CYAN] = {
        .base       = 0xff05ddff, // #05ddff
        .highlight  = 0xff65fffa, // #65fffa
        .shadow     = 0xff0592ff  // #0592ff
    },
    [BLOCK_BLUE] = {
        .base       = 0xff007bff, // #007bff
        .highlight  = 0xff00d2ff, // #00d2ff
        .shadow     = 0xff0048ff  // #0048ff
    },
    [BLOCK_PURPLE] = {
        .base       = 0xff753eda, // #753eda
        .highlight  = 0xffb866ec, // #b866ec
        .shadow     = 0xff4e32b5  // #4e32b5
    }
};

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

const float gravity_by_level[21] = {
    0.0f,     // unused
    0.01667f, // Level 1
    0.021017f,
    0.026977f,
    0.035256f,
    0.04693f,
    0.06361f,
    0.0879f,
    0.1236f,
    0.1775f,
    0.2598f,
    0.388f,
    0.59f,
    0.92f,
    1.46f,
    2.36f,
    3.91f,
    6.61f,
    11.43f,
    20.23f,
    36.6f
};
