#ifndef RAYCASTER_MAP_H
#define RAYCASTER_MAP_H

struct raycaster_map;

struct raycaster_map *rc_map_create(int map_width, int map_height, const int *const map_data);
int rc_map_get_wall(struct raycaster_map *map, int x, int y);
void rc_map_destroy(struct raycaster_map *map);

#endif
