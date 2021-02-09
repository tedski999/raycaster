#ifndef RAYCASTER_RENDERER_H
#define RAYCASTER_RENDERER_H

#include "window.h"
#include "map.h"
#include "texture.h"

struct raycaster_renderer;

struct raycaster_renderer *rc_renderer_create(struct raycaster_window *window, double aspect, int resolution, double fov, double wall_height, struct raycaster_texture **wall_textures);
void rc_renderer_set_dimensions(struct raycaster_renderer *renderer, int width, int height);
void rc_renderer_set_fov(struct raycaster_renderer *renderer, double fov);
void rc_renderer_set_resolution(struct raycaster_renderer *renderer, int resolution);
void rc_renderer_set_wall_height(struct raycaster_renderer *renderer, double wall_height);
void rc_renderer_set_wall_textures(struct raycaster_renderer *renderer, struct raycaster_texture **wall_textures);
void rc_renderer_draw(struct raycaster_renderer *renderer, struct raycaster_map *map, double camera_x, double camera_y, double camera_z, double camera_r);
void rc_renderer_destroy(struct raycaster_renderer *renderer);

#endif
