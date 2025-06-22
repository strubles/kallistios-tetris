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
    COLOR_ORANGE,
    COLOR_WHITE
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

typedef struct {
    TetrominoType type;
    BlockColor color;
    int size;               // 2, 3, or 4
    char symbol; // for debugging

    // Array representing what the tetromino looks like in its default state
    // This is read-only and is copied over to the dummy array in the game instance's active_tetro
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
    const TetrominoInfo *info;
    Rotation orientation;
    int set;

    // Array for holding the real current tetromino.
    // When a new tetromino is spawned, the relevant tetro array (TETRO_Z, TETRO_L, etc)
    // is copied into it from its TetrominoInfo.
    // Then, the dummy array can be rotated, without effecting how the next
    // tetromino of the same type will spawn in.
    int dummy[4][4]; // holder (rotated shapes are stored here)

    float fall_counter;
} Tetrodata;

typedef struct {
    int (*dummy)[4]; // should point to same dummy as the active tetromino so we dont have to copy it
    int left_x;
    int top_y;
} GhostTetro;

typedef struct {
    int current;
    int previous;

    int pressed;       // true while held
    int just_pressed;  // true only on the frame it was pressed
    int just_released; // true only on the frame it was released
} ButtonState;

typedef struct {
    int port;
    ButtonState dpad_up;
    ButtonState dpad_down;
    ButtonState dpad_left;
    ButtonState dpad_right;
    ButtonState button_a;
    ButtonState button_b;
    ButtonState button_x;
    ButtonState button_y;
    ButtonState button_start;

    // triggers range from 0 to 255
    int trigger_left;
    int trigger_right;

    // joystick ranges from -128 to 127 for each axis
    int joy_x;
    int joy_y;
} InputState;

typedef struct {
    int x, y;
} Point;

typedef struct {
    BlockColor field[24][12];

    // This variable an empty version of the field matrix. It is where the current active tetromino is put.
    // It is rendered to the screen directly on top of the main field matrix. When a tetromino is set,
    // it is copied over to the main field matrix in the same location, and the temp_field matrix is
    // cleared out for the next tetromino.
    // BlockColor temp_field[24][12];

    BlockColor hold_field[4][4];

    const TetrominoInfo *held_tetro;
    int hold_eligible;  // whether we will let the player perform a tetromino hold
    int has_drawn_new_tetro;
    long int score;
    int level;
    int line_clears;
    float fall_timer;
    // int falltime; // what we reset the timer to after every tetro fall

    // This holds data on the current active tetromino. It is continuously overwritten with the next
    // tetromino as the old tetromino get committed to the field matrix and doesn't need to be kept
    // track of anymore.
    Tetrodata active_tetro;
    GhostTetro ghost_tetro;
    int hard_drop_distance;

    int first_run;
    int loss;
    int move_timebuffer;
    int released_y_button;
    int released_x_button;
    int released_up_button;
    int released_ltrig;

    TetrominoType bag[7];
    int bag_index;

    InputState input;

    float gravity;

    Point ghost_tiles[16];
    int ghost_count;

} GameInstance;

#endif // TYPES_H
