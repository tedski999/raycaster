#include "input.h"

#define mouse_sensitivity 0.05

enum rc_input_state {
	RC_INPUT_STATE_UNPRESSED,
	RC_INPUT_STATE_PRESSED,
	RC_INPUT_STATE_HELD,
	RC_INPUT_STATE_RELEASED
};

static int mouse_x, mouse_y;
static int mouse_vx, mouse_vy;
static enum rc_input_state keys[rc_input_key_count];
static enum rc_input_state buttons[rc_input_button_count];

void rc_input_update(void) {
	for (int i = 0; i < rc_input_key_count; i++) {
		if (keys[i] == RC_INPUT_STATE_PRESSED)
			keys[i] = RC_INPUT_STATE_HELD;
		else if (keys[i] == RC_INPUT_STATE_RELEASED)
			keys[i] = RC_INPUT_STATE_UNPRESSED;
	}

	for (int i = 0; i < rc_input_button_count; i++) {
		if (buttons[i] == RC_INPUT_STATE_PRESSED)
			buttons[i] = RC_INPUT_STATE_HELD;
		else if (buttons[i] == RC_INPUT_STATE_RELEASED)
			buttons[i] = RC_INPUT_STATE_UNPRESSED;
	}

	mouse_vx = mouse_vy = 0;
}

bool rc_input_is_key_pressed(enum rc_input_key key) {
	return keys[key] == RC_INPUT_STATE_PRESSED;
}

bool rc_input_is_key_down(enum rc_input_key key) {
	return keys[key] == RC_INPUT_STATE_PRESSED || keys[key] == RC_INPUT_STATE_HELD;
}

bool rc_input_is_button_pressed(enum rc_input_button button) {
	return buttons[button] == RC_INPUT_STATE_PRESSED;
}

bool rc_input_is_button_down(enum rc_input_button button) {
	return buttons[button] == RC_INPUT_STATE_PRESSED || buttons[button] == RC_INPUT_STATE_HELD;
}

void rc_input_get_mouse_position(double *x, double *y) {
	*x = mouse_x;
	*y = mouse_y;
}

void rc_input_get_mouse_velocity(double *vx, double *vy) {
	*vx = mouse_vx * mouse_sensitivity;
	*vy = mouse_vy * mouse_sensitivity;
}

void rc_input_set_keyboard_input(enum rc_input_key key, bool state) {
	keys[key] = (state) ? RC_INPUT_STATE_PRESSED : RC_INPUT_STATE_RELEASED;
}

void rc_input_set_mouse_input(enum rc_input_button button, bool state) {
	buttons[button] = (state) ? RC_INPUT_STATE_PRESSED : RC_INPUT_STATE_RELEASED;
}

void rc_input_set_mouse_position(int x, int y) {
	mouse_vx = x - mouse_x;
	mouse_vy = y - mouse_y;
	mouse_x = x;
	mouse_y = y;
}
