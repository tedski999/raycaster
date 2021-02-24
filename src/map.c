#include "map.h"
#include "error.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

struct raycaster_map {
	int width, height;
	int *floor, *walls, *ceiling;
	unsigned char *lighting;
};

struct raycaster_map *rc_map_create(int map_width, int map_height, const int *const floor, const int *const walls, const int *const ceiling) {

	struct raycaster_map *map = malloc(sizeof *map);
	RC_ASSERT(map, "raycaster_map memory allocation");
	*map = (struct raycaster_map) { map_width, map_height };

	map->floor = malloc(sizeof (int) * map_width * map_height);
	map->walls = malloc(sizeof (int) * map_width * map_height);
	map->ceiling = malloc(sizeof (int) * map_width * map_height);
	map->lighting = calloc(3 * map_width * map_height, sizeof (unsigned char));
	RC_ASSERT(map->floor && map->walls && map->ceiling && map->lighting, "raycaster_map->data memory allocation");
	for (int i = 0; i < map_width * map_height; i++) {
		map->floor[i] = floor[i];
		map->walls[i] = walls[i];
		map->ceiling[i] = ceiling[i];
	}

	return map;
}

void rc_map_regenerate_lighting(struct raycaster_map *map, const struct raycaster_light *const lights, int lights_count) {

	// Ambient lighting
	// TODO: configurable
	for (int i = 0; i < map->width * map->height; i++) {
		map->lighting[3 * i + 0] = 0x10;
		map->lighting[3 * i + 1] = 0x10;
		map->lighting[3 * i + 2] = 0x10;
	}

	// Per-light grid distance lighting
	for (int i = 0; i < lights_count; i++) {

		// Skip disabled lights
		if (lights[i].range == 0)
			continue;

		// A boolean array for marking visited tiles
		bool *is_tile_visited = calloc(map->width * map->height, sizeof *is_tile_visited);
		is_tile_visited[lights[i].y * map->width + lights[i].x] = true;

		// Queue data structure for breadth first search
		int tile_queue_capacity = 4 * lights[i].range;
		int tile_queue_front_index = 0, tile_queue_back_index = 0;
		int *tile_queue = malloc(sizeof *tile_queue * tile_queue_capacity * 2);
		tile_queue[0] = lights[i].x;
		tile_queue[1] = lights[i].y;

		// Keep processing tiles until theres none left to process (either out of lighting range or all tiles have been visited)
		int distance = 0, distance_tiles_remaining = 1;
		while (distance <= lights[i].range && tile_queue_front_index <= tile_queue_back_index) {

			// Dequeue tile to process
			int dequeue_index = tile_queue_front_index++ % tile_queue_capacity * 2;
			int cur_tile_x = tile_queue[dequeue_index + 0];
			int cur_tile_y = tile_queue[dequeue_index + 1];

			// Apply lighting of tile
			int lighting_index = 3 * (cur_tile_y * map->width + cur_tile_x);
			double intensity = 1 - (double)distance / lights[i].range; // lighting attenuation linear component
			intensity = pow(intensity, lights[i].falloff);             // lighting attenuation exponential component
			map->lighting[lighting_index + 0] = fmin(0xff, map->lighting[lighting_index + 0] + lights[i].r * intensity);
			map->lighting[lighting_index + 1] = fmin(0xff, map->lighting[lighting_index + 1] + lights[i].g * intensity);
			map->lighting[lighting_index + 2] = fmin(0xff, map->lighting[lighting_index + 2] + lights[i].b * intensity);

			// Add valid surrounding tiles to the queue
			int adjacent_tile_step_x[4] = { 0, 1, 0, -1 };
			int adjacent_tile_step_y[4] = { 1, 0, -1, 0 };
			for (int j = 0; j < 4; j++) {
				int next_tile_x = cur_tile_x + adjacent_tile_step_x[j];
				int next_tile_y = cur_tile_y + adjacent_tile_step_y[j];

				// Don't process walls, tiles outside the map or already visited tiles
				if (rc_map_get_wall(map, next_tile_x, next_tile_y) != -1)
					continue;
				if (is_tile_visited[next_tile_y * map->width + next_tile_x])
					continue;

				// Enqueue tile
				is_tile_visited[next_tile_y * map->width + next_tile_x] = true;
				int enqueue_index = ++tile_queue_back_index % tile_queue_capacity * 2;
				tile_queue[enqueue_index + 0] = next_tile_x;
				tile_queue[enqueue_index + 1] = next_tile_y;
			}

			// Reduce the light intensity after all the tiles for this light intensity have been processed
			if (--distance_tiles_remaining <= 0) {
				distance_tiles_remaining = tile_queue_back_index - tile_queue_front_index + 1;
				distance++;
			}
		}

		free(is_tile_visited);
		free(tile_queue);
	}
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

void rc_map_get_lighting(struct raycaster_map *map, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b) {
	int lighting_index = 3 * (y * map->width + x);
	*r = map->lighting[lighting_index + 0];
	*g = map->lighting[lighting_index + 1];
	*b = map->lighting[lighting_index + 2];
}

void rc_map_destroy(struct raycaster_map *map) {
	free(map->floor);
	free(map->walls);
	free(map->ceiling);
	free(map->lighting);
	free(map);
}
