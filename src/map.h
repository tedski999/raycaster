#ifndef RAYCASTER_MAP_H
#define RAYCASTER_MAP_H

#include "light.h"

struct raycaster_map;

struct raycaster_map *rc_map_create(int map_width, int map_height, const int *const floor, const int *const walls, const int *const ceiling);
void rc_map_get_size(const struct raycaster_map *const map, int *width, int *height);
int rc_map_get_floor(const struct raycaster_map *const map, int x, int y);
int rc_map_get_wall(const struct raycaster_map *const map, int x, int y);
int rc_map_get_ceiling(const struct raycaster_map *const map, int x, int y);
void rc_map_generate_lighting(struct raycaster_map *map, unsigned char ambient_r, unsigned char ambient_g, unsigned char ambient_b, struct raycaster_light **lights, int lights_count);
void rc_map_get_lighting(const struct raycaster_map *const map, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b);
void rc_map_destroy(struct raycaster_map *map);

#endif
