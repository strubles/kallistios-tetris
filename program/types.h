#include <stdint.h>

#ifndef TYPES_H
#define TYPES_H

typedef enum {
    TETRO_NONE = 0,
    TETRO_I,
    TETRO_O,
    TETRO_T,
    TETRO_S,
    TETRO_Z,
    TETRO_J,
    TETRO_L,
    TETRO_COUNT,
} TetrominoType;

typedef enum {
    COLOR_NONE = 0,
    COLOR_CYAN,
    COLOR_YELLOW,
    COLOR_PURPLE,
    COLOR_GREEN,
    COLOR_RED,
    COLOR_BLUE,
    COLOR_ORANGE
} BlockColor;

typedef struct {
    uint8_t r, g, b, a;
} ColorRgba;

typedef enum {
    DEFAULT = 0,
    RIGHT,
    TWO,
    LEFT
} Rotation;

// fixme: remove later and replace with BlockColor
typedef enum {
    EMPTY = 0,
    RED = 1, // Z
    ORANGE = 2, // L
    YELLOW = 3, // O
    GREEN = 4, // S
    LIGHT_BLUE = 5, // I
    DARK_BLUE = 6, // J
    PURPLE = 7 // T
} color_id;

// fixme: remove and replace with Tetrodata
// typedef struct {
//     int top_y;
//     int left_x;
//     color_id type;
//     int size;
//     Rotation orientation;
//     int set;
//     int tetro_index;
// } Tetrodata;

typedef struct {
    TetrominoType type;
    BlockColor color;
    int size;               // 2, 3, or 4
    int shape[4][4];
    int perform_kicks; // bool
    int (*kicks_cw)[5][2];
    int (*kicks_ccw)[5][2];
    int initial_left_x;
    int initial_top_y;
} TetrominoInfo;

typedef struct {
    TetrominoType type;
    int left_x;
    int top_y;
    TetrominoInfo *info;
    Rotation orientation;
    int set;
    int dummy[4][4]; // holder (rotated shapes are stored here)
} Tetrodata;

typedef struct {
    BlockColor field[24][12];

    // This variable an empty version of the field matrix. It is where the current active tetromino is put.
    // It is rendered to the screen directly on top of the main field matrix. When a tetromino is set,
    // it is copied over to the main field matrix in the same location, and the temp_field matrix is
    // cleared out for the next tetromino.
    BlockColor temp_field[24][12];

    BlockColor hold_field[4][4];

    // Arrays for holding the real current tetromino.
    // When a new tetromino is spawned, the relevant tetro array (TETRO_Z, TETRO_L, etc)
    // is copied into it.
    // Then, these "dummy" arrays can be rotated, without effecting how the next
    // tetromino of the same type will spawn in.
    // There are 3 of them because while most tetrominos go in the 3x3 one, the I needs 4x4 and
    // the O needs 2x2.
	// color_id tetro_dummy_4x4[4][4];
	// color_id tetro_dummy_3x3[3][3];
	// color_id tetro_dummy_2x2[2][2];

    // fixme: possibly make held_tetro a ptr to a TetrominoInfo
    TetrominoInfo *held_tetro;
    int hold_eligible;  // whether we will let the player perform a tetromino hold
    int has_drawn_new_tetro;
    long int score;
    int level;
    int line_clears;
    int fall_timer;
    int falltime; // what we reset the timer to after every tetro fall
    Tetrodata active_tetro;
    int first_run;
    int loss;
    int move_timebuffer;
    int released_y_button;
    int released_x_button;
    int released_up_button;
    int released_ltrig;
} GameInstance;

#endif // TYPES_H