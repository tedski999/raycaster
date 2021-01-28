#include "map.h"
#include "error.h"
#include <stdlib.h>

struct raycaster_map {
	int width, height;
	int *data;
};

struct raycaster_map *rc_map_create(int map_width, int map_height, const int *const map_data) {
	struct raycaster_map *map = malloc(sizeof *map);
	RC_ASSERT(map, "raycaster_map memory allocation");
	*map = (struct raycaster_map) { map_width, map_height };
	map->data = malloc(sizeof (int) * map_width * map_height);
	RC_ASSERT(map->data, "raycaster_map->data memory allocation");
	for (int i = 0; i < map_width * map_height; i++)
		map->data[i] = map_data[i];
	return map;
}

int rc_map_get_wall(struct raycaster_map *map, int x, int y) {
	if (x < 0 || x >= map->width || y < 0 || y >= map->height)
		return -1;
	return map->data[y * map->width + x];
}

void rc_map_destroy(struct raycaster_map *map) {
	free(map->data);
	free(map);
}

