#include "map.h"
#include "error.h"
#include <stdlib.h>

struct raycaster_map {
	int width, height;
	int *floor, *walls, *ceiling;
};

struct raycaster_map *rc_map_create(int map_width, int map_height, const int *const floor, const int *const walls, const int *const ceiling) {

	struct raycaster_map *map = malloc(sizeof *map);
	RC_ASSERT(map, "raycaster_map memory allocation");
	*map = (struct raycaster_map) { map_width, map_height };

	map->floor = malloc(sizeof (int) * map_width * map_height);
	map->walls = malloc(sizeof (int) * map_width * map_height);
	map->ceiling = malloc(sizeof (int) * map_width * map_height);
	RC_ASSERT(map->floor && map->walls && map->ceiling, "raycaster_map->data memory allocation");

	for (int i = 0; i < map_width * map_height; i++) {
		map->floor[i] = floor[i];
		map->walls[i] = walls[i];
		map->ceiling[i] = ceiling[i];
	}

	return map;
}

void rc_map_get_size(struct raycaster_map *map, int *width, int *height) {
	*width = map->width;
	*height = map->height;
}

int rc_map_get_floor(struct raycaster_map *map, int x, int y) {
	if (x < 0 || x >= map->width || y < 0 || y >= map->height)
		return 0;
	return map->floor[y * map->width + x];
}

int rc_map_get_wall(struct raycaster_map *map, int x, int y) {
	if (x < 0 || x >= map->width || y < 0 || y >= map->height)
		return 0;
	return map->walls[y * map->width + x];
}

int rc_map_get_ceiling(struct raycaster_map *map, int x, int y) {
	if (x < 0 || x >= map->width || y < 0 || y >= map->height)
		return 0;
	return map->ceiling[y * map->width + x];
}

void rc_map_destroy(struct raycaster_map *map) {
	free(map->floor);
	free(map->walls);
	free(map->ceiling);
	free(map);
}
