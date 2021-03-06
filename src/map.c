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

void rc_map_get_size(const struct raycaster_map *const map, int *width, int *height) {
	*width = map->width;
	*height = map->height;
}

int rc_map_get_floor(const struct raycaster_map *const map, int x, int y) {
	if (x < 0 || x >= map->width || y < 0 || y >= map->height)
		return 0;
	return map->floor[y * map->width + x];
}

int rc_map_get_wall(const struct raycaster_map *const map, int x, int y) {
	if (x < 0 || x >= map->width || y < 0 || y >= map->height)
		return 0;
	return map->walls[y * map->width + x];
}

int rc_map_get_ceiling(const struct raycaster_map *const map, int x, int y) {
	if (x < 0 || x >= map->width || y < 0 || y >= map->height)
		return 0;
	return map->ceiling[y * map->width + x];
}

void rc_map_generate_lighting(struct raycaster_map *map, unsigned char ambient_r, unsigned char ambient_g, unsigned char ambient_b, struct raycaster_light **lights, int lights_count) {

	// Ambient lighting
	for (int i = 0; i < map->width * map->height; i++) {
		map->lighting[3 * i + 0] = ambient_r;
		map->lighting[3 * i + 1] = ambient_g;
		map->lighting[3 * i + 2] = ambient_b;
	}

	// Per-light grid distance lighting
	// TODO: Might be far more performant to have each light only recalculate its local lightmap when its properties change.
	//       Then we only have to combine their local lightmaps into a global lightmap here every frame.
	//       A lights local lightmap is only needs to be 4*range in size (if we use fancy indexing, else its range^2).
	for (int i = 0; i < lights_count; i++) {

		int light_x, light_y;
		unsigned char light_r, light_g, light_b;
		int light_range;
		double light_falloff;
		rc_light_get_position(lights[i], &light_x, &light_y);
		rc_light_get_color(lights[i], &light_r, &light_g, &light_b);
		rc_light_get_lighting(lights[i], &light_range, &light_falloff);

		// Skip disabled lights
		if (light_range == 0)
			continue;

		// A boolean array for marking visited tiles
		bool *is_tile_visited = calloc(map->width * map->height, sizeof *is_tile_visited);
		RC_ASSERT(is_tile_visited, "rc_map_regenerate_lighting is_tile_visited memory allocation");
		is_tile_visited[light_y * map->width + light_x] = true;

		// Queue data structure for breadth first search
		int tile_queue_capacity = 4 * light_range;
		int tile_queue_front_index = 0, tile_queue_back_index = 0;
		int *tile_queue = malloc(sizeof *tile_queue * tile_queue_capacity * 2);
		RC_ASSERT(tile_queue, "rc_map_regenerate_lighting tile_queue memory allocation");
		tile_queue[0] = light_x;
		tile_queue[1] = light_y;

		// Keep processing tiles until theres none left to process (either out of lighting range or all tiles have been visited)
		int distance = 0, distance_tiles_remaining = 1;
		while (distance <= light_range && tile_queue_front_index <= tile_queue_back_index) {

			// Dequeue tile to process
			int dequeue_index = tile_queue_front_index++ % tile_queue_capacity * 2;
			int cur_tile_x = tile_queue[dequeue_index + 0];
			int cur_tile_y = tile_queue[dequeue_index + 1];

			// Apply lighting of tile
			int lighting_index = 3 * (cur_tile_y * map->width + cur_tile_x);
			double intensity = 1 - (double)distance / light_range; // lighting attenuation linear component
			intensity = pow(intensity, light_falloff);             // lighting attenuation exponential component
			map->lighting[lighting_index + 0] = fmin(0xff, map->lighting[lighting_index + 0] + light_r * intensity);
			map->lighting[lighting_index + 1] = fmin(0xff, map->lighting[lighting_index + 1] + light_g * intensity);
			map->lighting[lighting_index + 2] = fmin(0xff, map->lighting[lighting_index + 2] + light_b * intensity);

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

void rc_map_get_lighting(const struct raycaster_map *const map, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b) {
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
