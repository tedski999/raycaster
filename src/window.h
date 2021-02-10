#ifndef RAYCASTER_WINDOW_H
#define RAYCASTER_WINDOW_H

#include <stdbool.h>

struct raycaster_window;

struct raycaster_window *rc_window_create(const char *const title, int width, int height, bool is_resizable, bool is_cursor_disabled, bool is_vsync_enabled);
void rc_window_set_vsync_enabled(struct raycaster_window *window, bool is_vsync_enabled);
void rc_window_set_renderer(struct raycaster_window *window, void *renderer);
void rc_window_set_as_context(struct raycaster_window *window);
bool rc_window_should_close(struct raycaster_window *window);
void rc_window_update(struct raycaster_window *window);
void rc_window_render(struct raycaster_window *window);
void rc_window_destroy(struct raycaster_window *window);

#endif
