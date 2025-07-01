#include <kos.h>

#include <types.h>

void update_button_state(ButtonState *btn, int is_down_now) {
    btn->previous = btn->current;
    btn->current = is_down_now;

    btn->pressed = is_down_now;
    btn->just_pressed = (!btn->previous && is_down_now);
    btn->just_released = (btn->previous && !is_down_now);
}

void update_inputs(InputState* input, int port_index) {
    maple_device_t *cont = maple_enum_type(port_index, MAPLE_FUNC_CONTROLLER);
    if (!cont) return;

    cont_state_t *state = (cont_state_t *) maple_dev_status(cont);

    update_button_state(&input->dpad_up, state->buttons & CONT_DPAD_UP);
    update_button_state(&input->dpad_down, state->buttons & CONT_DPAD_DOWN);
    update_button_state(&input->dpad_left, state->buttons & CONT_DPAD_LEFT);
    update_button_state(&input->dpad_right, state->buttons & CONT_DPAD_RIGHT);
    update_button_state(&input->button_a, state->buttons & CONT_A);
    update_button_state(&input->button_b, state->buttons & CONT_B);
    update_button_state(&input->button_x, state->buttons & CONT_X);
    update_button_state(&input->button_y, state->buttons & CONT_Y);
    update_button_state(&input->button_start, state->buttons & CONT_START);

    input->trigger_left = state->ltrig;
    input->trigger_right = state->rtrig;
    input->joy_x = state->joyx;
    input->joy_y = state->joyy;
}
