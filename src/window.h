#ifndef RC_WINDOW_H
#define RC_WINDOW_H

#include <stdbool.h>

struct rc_window;
struct rc_renderer;

struct rc_window *rc_window_create(const char *title, int width, int height, bool is_resizable, bool is_cursor_disabled, bool is_vsync_enabled);
void rc_window_set_vsync_enabled(const struct rc_window *window, bool is_vsync_enabled);
void rc_window_set_renderer(const struct rc_window *window, const struct rc_renderer *renderer);
void rc_window_set_as_context(const struct rc_window *window);
bool rc_window_should_close(const struct rc_window *window);
void rc_window_update(const struct rc_window *window);
void rc_window_render(const struct rc_window *window);
void rc_window_destroy(struct rc_window *window);

#endif
