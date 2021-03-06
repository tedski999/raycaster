#ifndef RAYCASTER_INPUT_H
#define RAYCASTER_INPUT_H

#include <stdbool.h>

#define INPUT_BUTTON_LEFT   INPUT_BUTTON_1
#define INPUT_BUTTON_RIGHT  INPUT_BUTTON_2
#define INPUT_BUTTON_MIDDLE INPUT_BUTTON_3

enum input_key {
	INPUT_KEY_SPACE = 1, INPUT_KEY_ENTER, INPUT_KEY_BACKSPACE,
	INPUT_KEY_TAB, INPUT_KEY_ESCAPE,
	INPUT_KEY_SHIFT, INPUT_KEY_RIGHT_SHIFT,
	INPUT_KEY_CONTROL, INPUT_KEY_RIGHT_CONTROL,
	INPUT_KEY_ALT, INPUT_KEY_RIGHT_ALT,
	INPUT_KEY_LEFT_BRACKET, INPUT_KEY_RIGHT_BRACKET,
	INPUT_KEY_SEMICOLON, INPUT_KEY_APOSTROPHE,
	INPUT_KEY_COMMA, INPUT_KEY_PERIOD,
	INPUT_KEY_SLASH, INPUT_KEY_BACKSLASH,
	INPUT_KEY_MINUS, INPUT_KEY_EQUALS,
	INPUT_KEY_INSERT, INPUT_KEY_DELETE,
	INPUT_KEY_HOME, INPUT_KEY_END,
	INPUT_KEY_PAGE_UP, INPUT_KEY_PAGE_DOWN,
	INPUT_KEY_RIGHT, INPUT_KEY_LEFT,
	INPUT_KEY_DOWN, INPUT_KEY_UP,
	INPUT_KEY_0, INPUT_KEY_1,
	INPUT_KEY_2, INPUT_KEY_3,
	INPUT_KEY_4, INPUT_KEY_5,
	INPUT_KEY_6, INPUT_KEY_7,
	INPUT_KEY_8, INPUT_KEY_9,
	INPUT_KEY_A, INPUT_KEY_B,
	INPUT_KEY_C, INPUT_KEY_D,
	INPUT_KEY_E, INPUT_KEY_F,
	INPUT_KEY_G, INPUT_KEY_H,
	INPUT_KEY_I, INPUT_KEY_J,
	INPUT_KEY_K, INPUT_KEY_L,
	INPUT_KEY_M, INPUT_KEY_N,
	INPUT_KEY_O, INPUT_KEY_P,
	INPUT_KEY_Q, INPUT_KEY_R,
	INPUT_KEY_S, INPUT_KEY_T,
	INPUT_KEY_U, INPUT_KEY_V,
	INPUT_KEY_W, INPUT_KEY_X,
	INPUT_KEY_Y, INPUT_KEY_Z,
	input_key_count
};

enum input_button {
	INPUT_BUTTON_1,
	INPUT_BUTTON_2,
	INPUT_BUTTON_3,
	INPUT_BUTTON_4,
	INPUT_BUTTON_5,
	INPUT_BUTTON_6,
	INPUT_BUTTON_7,
	INPUT_BUTTON_8,
	input_button_count
};

void rc_input_update();
bool rc_input_is_key_pressed(enum input_key key);
bool rc_input_is_key_down(enum input_key key);
bool rc_input_is_button_pressed(enum input_button button);
bool rc_input_is_button_down(enum input_button button);
void rc_input_get_mouse_position(double *x, double *y);
void rc_input_get_mouse_velocity(double *vx, double *vy);
void rc_input_set_keyboard_input(enum input_key key, bool state);
void rc_input_set_mouse_input(enum input_button button, bool state);
void rc_input_set_mouse_position(int x, int y);

#endif
