#ifndef RAYCASTER_RENDERER_H
#define RAYCASTER_RENDERER_H

#include "window.h"
#include "map.h"
#include "entity.h"
#include "texture.h"

struct raycaster_renderer;

struct raycaster_renderer *rc_renderer_create(struct raycaster_window *window, double aspect, int resolution, double fov, struct raycaster_texture **wall_textures);
void rc_renderer_set_dimensions(struct raycaster_renderer *renderer, int width, int height);
void rc_renderer_set_fov(struct raycaster_renderer *renderer, double fov);
void rc_renderer_set_resolution(struct raycaster_renderer *renderer, int resolution);
void rc_renderer_set_wall_textures(struct raycaster_renderer *renderer, struct raycaster_texture **wall_textures);
void rc_renderer_draw(struct raycaster_renderer *renderer, struct raycaster_map *map, struct raycaster_entity **entities, int entities_count, struct raycaster_entity *camera);
void rc_renderer_destroy(struct raycaster_renderer *renderer);

#endif
