#include "window.h"
#include "renderer.h"
#include "player.h"
#include "map.h"
#include <stdbool.h>

#define DEG2RAD(degree) (degree * 3.14152 / 180)

#define window_title "raycaster"
#define window_width 640
#define window_height 480
#define window_aspect (16.0 / 9.0)

int main(int argc, char **argv) {

	int pixelation = 5;
	double fov = DEG2RAD(60);
	double wallheight = 1.0;

	// TODO: map should have 3 layers: floor, walls, ceiling
	const int map_width = 24, map_height = 24, map_data[24*24] = {
		4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,7,7,7,7,7,7,7,7,
		4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7,
		4,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,
		4,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,
		4,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7,
		4,0,4,0,0,0,0,5,5,5,5,5,5,5,5,5,7,7,0,7,7,7,7,7,
		4,0,5,0,0,0,0,5,0,5,0,5,0,5,0,5,7,0,0,0,7,7,7,1,
		4,0,6,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8,
		4,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,1,
		4,0,8,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8,
		4,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,7,7,7,1,
		4,0,0,0,0,0,0,5,5,5,5,0,5,5,5,5,7,7,7,7,7,7,7,1,
		6,6,6,6,6,6,6,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6,
		8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,
		6,6,6,6,6,6,0,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6,
		4,4,4,4,4,4,0,4,4,4,6,0,6,2,2,2,2,2,2,2,3,3,3,3,
		4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2,
		4,0,0,0,0,0,0,0,0,0,0,0,6,2,0,0,5,0,0,2,0,0,0,2,
		4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2,
		4,0,6,0,6,0,0,0,0,4,6,0,0,0,0,0,5,0,0,0,0,0,0,2,
		4,0,0,5,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2,
		4,0,6,0,6,0,0,0,0,4,6,0,6,2,0,0,5,0,0,2,0,0,0,2,
		4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2,
		4,4,4,4,4,4,4,4,4,4,1,1,1,2,2,2,2,2,2,3,3,3,3,3
	};

	// Open a window and assign a raycast renderer to it
	struct raycaster_window *window = window_create(window_title, window_width, window_height, true, false);
	struct raycaster_renderer *renderer = renderer_create(window_width, window_height, window_aspect, fov, pixelation, wallheight);
	window_set_renderer(window, renderer);
	window_update(window);

	// Game-related objects
	struct raycaster_map *map = map_create(map_width, map_height, map_data);
	struct raycaster_player *player = player_create(window, map, 11.5, 7.5, DEG2RAD(90));

	// Main game loop
	bool is_running = true;
	while (is_running) {

		// Update
		window_update(window);
		player_update(player);
		if (window_should_close(window))
			is_running = false;

		// Debug input
		if (window_is_key_down(window, INPUT_KEY_Q)) renderer_set_fov(renderer, fov -= 0.01);
		if (window_is_key_down(window, INPUT_KEY_E)) renderer_set_fov(renderer, fov += 0.01);
		if (window_is_key_down(window, INPUT_KEY_Z)) renderer_set_pixelation(renderer, pixelation += 1);
		if (window_is_key_down(window, INPUT_KEY_X)) if (pixelation > 1) renderer_set_pixelation(renderer, pixelation -= 1);
		if (window_is_key_down(window, INPUT_KEY_COMMA)) renderer_set_wallheight(renderer, wallheight -= 0.01);
		if (window_is_key_down(window, INPUT_KEY_PERIOD)) renderer_set_wallheight(renderer, wallheight += 0.01);
		if (window_is_key_down(window, INPUT_KEY_ESCAPE)) is_running = false;

		// Render
		double player_x, player_y, player_r;
		player_get_transform(player, &player_x, &player_y, &player_r);
		window_set_as_context(window);
		renderer_draw(renderer, map, player_x, player_y, player_r);
		window_render(window);
	}

	// Cleanup
	map_destroy(map);
	player_destroy(player);
	renderer_destroy(renderer);
	window_destroy(window);
}