#ifndef MAIN_H
#define MAIN_H

#include "types.h"

void init();
void init_game_instance(GameInstance* game);
int check_valid_state(GameInstance* game);
void clear_line(GameInstance* game, int rownum);
void check_lines(GameInstance* game);
void shuffle_bag(GameInstance* game);
void update_pause(GameInstance* game);
void advance_game_logic(GameInstance* game);
int main();

#endif // MAIN_H
