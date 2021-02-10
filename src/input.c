#include "input.h"

#define mouse_sensitivity 0.05

enum input_state {
	INPUT_STATE_UNPRESSED,
	INPUT_STATE_PRESSED,
	INPUT_STATE_HELD,
	INPUT_STATE_RELEASED
};

static int mouse_x, mouse_y;
static int mouse_vx, mouse_vy;
static enum input_state keys[input_key_count];
static enum input_state buttons[input_button_count];

void rc_input_update() {
	for (int i = 0; i < input_key_count; i++) {
		if (keys[i] == INPUT_STATE_PRESSED)
			keys[i] = INPUT_STATE_HELD;
		else if (keys[i] == INPUT_STATE_RELEASED)
			keys[i] = INPUT_STATE_UNPRESSED;
	}

	for (int i = 0; i < input_button_count; i++) {
		if (buttons[i] == INPUT_STATE_PRESSED)
			buttons[i] = INPUT_STATE_HELD;
		else if (buttons[i] == INPUT_STATE_RELEASED)
			buttons[i] = INPUT_STATE_UNPRESSED;
	}

	mouse_vx = mouse_vy = 0;
}

bool rc_input_is_key_pressed(enum input_key key) {
	return keys[key] == INPUT_STATE_PRESSED;
}

bool rc_input_is_key_down(enum input_key key) {
	return keys[key] == INPUT_STATE_PRESSED || keys[key] == INPUT_STATE_HELD;
}

bool rc_input_is_button_pressed(enum input_button button) {
	return buttons[button] == INPUT_STATE_PRESSED;
}

bool rc_input_is_button_down(enum input_button button) {
	return buttons[button] == INPUT_STATE_PRESSED || buttons[button] == INPUT_STATE_HELD;
}

void rc_input_get_mouse_position(double *const x, double *const y) {
	*x = mouse_x;
	*y = mouse_y;
}

void rc_input_get_mouse_velocity(double *const vx, double *const vy) {
	*vx = mouse_vx * mouse_sensitivity;
	*vy = mouse_vy * mouse_sensitivity;
}

void rc_input_set_keyboard_input(enum input_key key, bool state) {
	keys[key] = (state) ? INPUT_STATE_PRESSED : INPUT_STATE_RELEASED;
}

void rc_input_set_mouse_input(enum input_button button, bool state) {
	buttons[button] = (state) ? INPUT_STATE_PRESSED : INPUT_STATE_RELEASED;
}

void rc_input_set_mouse_position(int x, int y) {
	mouse_vx = x - mouse_x;
	mouse_vy = y - mouse_y;
	mouse_x = x;
	mouse_y = y;
}
