#ifndef MAIN_H
#define MAIN_H

#include "types.h"

void shuffle_bag(GameInstance* game);
void rotate_tetro_counterclockwise(GameInstance* game);
void generate_new_tetro(GameInstance* game);
int find_hard_drop_distance(GameInstance* game);
void update_ghost_piece(GameInstance* game);

#endif
