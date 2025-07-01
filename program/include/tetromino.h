#ifndef TETROMINO_H
#define TETROMINO_H

#include <types.h>

void init_new_tetro(GameInstance* game, TetrominoType type);
void commit_active_tetro(GameInstance* game);
void tetro_left(GameInstance* game);
void tetro_right(GameInstance* game);
void update_ghost_piece(GameInstance* game);
void soft_drop(GameInstance* game, int award_score, int commit);
void hard_drop(GameInstance* game);
int find_hard_drop_distance(GameInstance* game);
void transpose_active_tetro(GameInstance* game);
void reverse_active_tetro_row(GameInstance* game, int row);
void update_orientation_cw(Tetrodata* tetro);
void update_orientation_ccw(Tetrodata* tetro);
void move_tetro_like_this(Tetrodata* tetro, int x, int y);
void swap(int* arg1, int* arg2);
void rotate_tetro_clockwise(GameInstance* game);
void rotate_tetro_counterclockwise(GameInstance* game);
void hold_tetromino(GameInstance* game);
int process_tetro_movement(GameInstance* game);
void generate_new_tetro(GameInstance* game);
void process_tetro_fall(GameInstance *game);

#endif // TETROMINO_H