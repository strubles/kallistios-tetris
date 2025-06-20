#ifndef INPUT_H
#define INPUT_H

#include "types.h"

void update_button_state(ButtonState *btn, int is_down_now);
void update_inputs(InputState* input, int port_index);

#endif
