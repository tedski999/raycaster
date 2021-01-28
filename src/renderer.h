#ifndef RAYCASTER_RENDERER_H
#define RAYCASTER_RENDERER_H

#include "map.h"

struct raycaster_renderer;

struct raycaster_renderer *rc_renderer_create(int width, int height, double aspect, double fov, int pixelation, double wallheight);
void rc_renderer_set_dimensions(struct raycaster_renderer *renderer, int width, int height);
void rc_renderer_set_fov(struct raycaster_renderer *renderer, double fov);
void rc_renderer_set_pixelation(struct raycaster_renderer *renderer, int pixelation);
void rc_renderer_set_wallheight(struct raycaster_renderer *renderer, double wallheight);
void rc_renderer_draw(struct raycaster_renderer *renderer, struct raycaster_map *map, double camera_x, double camera_y, double camera_r);
void rc_renderer_destroy(struct raycaster_renderer *renderer);

#endif
