#ifndef RAYCASTER_WINDOW_H
#define RAYCASTER_WINDOW_H

#include <stdbool.h>

struct raycaster_window;
struct raycaster_renderer;

struct raycaster_window *rc_window_create(const char *title, int width, int height, bool is_resizable, bool is_cursor_disabled, bool is_vsync_enabled);
void rc_window_set_vsync_enabled(const struct raycaster_window *window, bool is_vsync_enabled);
void rc_window_set_renderer(const struct raycaster_window *window, const struct raycaster_renderer *renderer);
void rc_window_set_as_context(const struct raycaster_window *window);
bool rc_window_should_close(const struct raycaster_window *window);
void rc_window_update(const struct raycaster_window *window);
void rc_window_render(const struct raycaster_window *window);
void rc_window_destroy(struct raycaster_window *window);

#endif
