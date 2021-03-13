#ifndef RC_INPUT_H
#define RC_INPUT_H

#include <stdbool.h>

#define INPUT_BUTTON_LEFT   INPUT_BUTTON_1
#define INPUT_BUTTON_RIGHT  INPUT_BUTTON_2
#define INPUT_BUTTON_MIDDLE INPUT_BUTTON_3

enum rc_input_key {
	RC_INPUT_KEY_SPACE = 1, RC_INPUT_KEY_ENTER, RC_INPUT_KEY_BACKSPACE,
	RC_INPUT_KEY_TAB, RC_INPUT_KEY_ESCAPE,
	RC_INPUT_KEY_SHIFT, RC_INPUT_KEY_RIGHT_SHIFT,
	RC_INPUT_KEY_CONTROL, RC_INPUT_KEY_RIGHT_CONTROL,
	RC_INPUT_KEY_ALT, RC_INPUT_KEY_RIGHT_ALT,
	RC_INPUT_KEY_LEFT_BRACKET, RC_INPUT_KEY_RIGHT_BRACKET,
	RC_INPUT_KEY_SEMICOLON, RC_INPUT_KEY_APOSTROPHE,
	RC_INPUT_KEY_COMMA, RC_INPUT_KEY_PERIOD,
	RC_INPUT_KEY_SLASH, RC_INPUT_KEY_BACKSLASH,
	RC_INPUT_KEY_MINUS, RC_INPUT_KEY_EQUALS,
	RC_INPUT_KEY_INSERT, RC_INPUT_KEY_DELETE,
	RC_INPUT_KEY_HOME, RC_INPUT_KEY_END,
	RC_INPUT_KEY_PAGE_UP, RC_INPUT_KEY_PAGE_DOWN,
	RC_INPUT_KEY_RIGHT, RC_INPUT_KEY_LEFT,
	RC_INPUT_KEY_DOWN, RC_INPUT_KEY_UP,
	RC_INPUT_KEY_0, RC_INPUT_KEY_1,
	RC_INPUT_KEY_2, RC_INPUT_KEY_3,
	RC_INPUT_KEY_4, RC_INPUT_KEY_5,
	RC_INPUT_KEY_6, RC_INPUT_KEY_7,
	RC_INPUT_KEY_8, RC_INPUT_KEY_9,
	RC_INPUT_KEY_A, RC_INPUT_KEY_B,
	RC_INPUT_KEY_C, RC_INPUT_KEY_D,
	RC_INPUT_KEY_E, RC_INPUT_KEY_F,
	RC_INPUT_KEY_G, RC_INPUT_KEY_H,
	RC_INPUT_KEY_I, RC_INPUT_KEY_J,
	RC_INPUT_KEY_K, RC_INPUT_KEY_L,
	RC_INPUT_KEY_M, RC_INPUT_KEY_N,
	RC_INPUT_KEY_O, RC_INPUT_KEY_P,
	RC_INPUT_KEY_Q, RC_INPUT_KEY_R,
	RC_INPUT_KEY_S, RC_INPUT_KEY_T,
	RC_INPUT_KEY_U, RC_INPUT_KEY_V,
	RC_INPUT_KEY_W, RC_INPUT_KEY_X,
	RC_INPUT_KEY_Y, RC_INPUT_KEY_Z,
	rc_input_key_count
};

enum rc_input_button {
	RC_INPUT_BUTTON_1,
	RC_INPUT_BUTTON_2,
	RC_INPUT_BUTTON_3,
	RC_INPUT_BUTTON_4,
	RC_INPUT_BUTTON_5,
	RC_INPUT_BUTTON_6,
	RC_INPUT_BUTTON_7,
	RC_INPUT_BUTTON_8,
	rc_input_button_count
};

void rc_input_update(void);
bool rc_input_is_key_pressed(enum rc_input_key key);
bool rc_input_is_key_down(enum rc_input_key key);
bool rc_input_is_button_pressed(enum rc_input_button button);
bool rc_input_is_button_down(enum rc_input_button button);
void rc_input_get_mouse_position(double *x, double *y);
void rc_input_get_mouse_velocity(double *vx, double *vy);
void rc_input_set_keyboard_input(enum rc_input_key key, bool state);
void rc_input_set_mouse_input(enum rc_input_button button, bool state);
void rc_input_set_mouse_position(int x, int y);

#endif
