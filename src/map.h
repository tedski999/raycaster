#ifndef RAYCASTER_MAP_H
#define RAYCASTER_MAP_H

struct raycaster_map;

struct raycaster_map *map_create(int map_width, int map_height, const int *const map_data);
int map_get_wall(struct raycaster_map *map, int x, int y);
void map_destroy(struct raycaster_map *map);

#endif
