#ifndef RAYCASTER_WINDOW_H
#define RAYCASTER_WINDOW_H

#include <stdbool.h>

enum input_key {
	INPUT_KEY_SPACE = 32,
	INPUT_KEY_APOSTROPHE = 39,
	INPUT_KEY_COMMA = 44,
	INPUT_KEY_MINUS = 45,
	INPUT_KEY_PERIOD = 46,
	INPUT_KEY_SLASH = 47,
	INPUT_KEY_0 = 48,
	INPUT_KEY_1 = 49,
	INPUT_KEY_2 = 50,
	INPUT_KEY_3 = 51,
	INPUT_KEY_4 = 52,
	INPUT_KEY_5 = 53,
	INPUT_KEY_6 = 54,
	INPUT_KEY_7 = 55,
	INPUT_KEY_8 = 56,
	INPUT_KEY_9 = 57,
	INPUT_KEY_SEMICOLON = 59,
	INPUT_KEY_EQUAL = 61,
	INPUT_KEY_A = 65,
	INPUT_KEY_B = 66,
	INPUT_KEY_C = 67,
	INPUT_KEY_D = 68,
	INPUT_KEY_E = 69,
	INPUT_KEY_F = 70,
	INPUT_KEY_G = 71,
	INPUT_KEY_H = 72,
	INPUT_KEY_I = 73,
	INPUT_KEY_J = 74,
	INPUT_KEY_K = 75,
	INPUT_KEY_L = 76,
	INPUT_KEY_M = 77,
	INPUT_KEY_N = 78,
	INPUT_KEY_O = 79,
	INPUT_KEY_P = 80,
	INPUT_KEY_Q = 81,
	INPUT_KEY_R = 82,
	INPUT_KEY_S = 83,
	INPUT_KEY_T = 84,
	INPUT_KEY_U = 85,
	INPUT_KEY_V = 86,
	INPUT_KEY_W = 87,
	INPUT_KEY_X = 88,
	INPUT_KEY_Y = 89,
	INPUT_KEY_Z = 90,
	INPUT_KEY_LEFT_BRACKET = 91,
	INPUT_KEY_BACKSLASH = 92,
	INPUT_KEY_RIGHT_BRACKET = 93,
	INPUT_KEY_ESCAPE = 256,
	INPUT_KEY_ENTER = 257,
	INPUT_KEY_TAB = 258,
	INPUT_KEY_BACKSPACE = 259,
	INPUT_KEY_INSERT = 260,
	INPUT_KEY_DELETE = 261,
	INPUT_KEY_RIGHT = 262,
	INPUT_KEY_LEFT = 263,
	INPUT_KEY_DOWN = 264,
	INPUT_KEY_UP = 265,
	INPUT_KEY_PAGE_UP = 266,
	INPUT_KEY_PAGE_DOWN = 267,
	INPUT_KEY_HOME = 268,
	INPUT_KEY_END = 269
};

struct raycaster_window;

struct raycaster_window *rc_window_create(const char *const title, int width, int height, bool is_resizable, bool is_cursor_disabled, bool is_vsync_enabled);
void rc_window_set_vsync_enabled(struct raycaster_window *window, bool is_vsync_enabled);
void rc_window_set_renderer(struct raycaster_window *window, void *renderer);
void rc_window_set_as_context(struct raycaster_window *window);
bool rc_window_is_key_down(struct raycaster_window *window, enum input_key key);
void rc_window_get_mouse_position(struct raycaster_window *window, double *const x, double *const y);
void *rc_window_get_load_proc(struct raycaster_window *window);
bool rc_window_should_close(struct raycaster_window *window);
void rc_window_update(struct raycaster_window *window);
void rc_window_render(struct raycaster_window *window);
void rc_window_destroy(struct raycaster_window *window);

#endif
