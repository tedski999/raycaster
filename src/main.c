#include "window.h"
#include "renderer.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#define DEG2RAD(degree) (degree * 3.14152 / 180)

#define window_title "raycaster"
#define window_width 640
#define window_height 480
#define window_aspect (16.0 / 9.0)

#define player_accel 0.01
#define player_drag 1.2
#define player_turn_speed 0.04
#define mouse_sensitivity 0.01

static int pixelation = 5;
static double fov = DEG2RAD(60);
static double wallheight = 1.0;

int main(int argc, char **argv) {

	// TODO: map should have 3 layers: floor, walls, ceiling
	const int map_width = 24, map_height = 24, map[24*24] = {
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

	double player_x = 11.5, player_y = 7.5;
	double player_vx = 0, player_vy = 0;
	double player_r = DEG2RAD(90);
	double prev_mouse_x = 0, prev_mouse_y = 0;

	// Open a window and assign a raycast renderer to it
	struct raycaster_window *window = window_create(window_title, window_width, window_height, true, false);
	struct raycaster_renderer *renderer = renderer_create(window_width, window_height, window_aspect, fov, pixelation, wallheight);
	window_set_resizable_renderer(window, renderer);

	// Process initial resizing events and cursor starting position
	window_update(window);
	window_get_mouse_position(window, &prev_mouse_x, &prev_mouse_y);

	// Main game loop
	bool is_running = true;
	while (is_running) {

		window_update(window);

		// Mouse velocity
		double mouse_x, mouse_y, mouse_vx, mouse_vy;
		window_get_mouse_position(window, &mouse_x, &mouse_y);
		mouse_vx = mouse_x - prev_mouse_x;
		mouse_vy = mouse_y - prev_mouse_y;
		prev_mouse_x = mouse_x;
		prev_mouse_y = mouse_y;

		// Player rotation
		player_r += mouse_vx * mouse_sensitivity;
		if (window_is_key_down(window, INPUT_KEY_RIGHT)) player_r += player_turn_speed;
		if (window_is_key_down(window, INPUT_KEY_LEFT)) player_r -= player_turn_speed;

		// Player movement
		if (window_is_key_down(window, INPUT_KEY_W)) { player_vx += cos(player_r) * player_accel; player_vy += sin(player_r) * player_accel; }
		if (window_is_key_down(window, INPUT_KEY_S)) { player_vx -= cos(player_r) * player_accel; player_vy -= sin(player_r) * player_accel; }
		if (window_is_key_down(window, INPUT_KEY_D)) { player_vx -= sin(player_r) * player_accel; player_vy += cos(player_r) * player_accel; }
		if (window_is_key_down(window, INPUT_KEY_A)) { player_vx += sin(player_r) * player_accel; player_vy -= cos(player_r) * player_accel; }
		if (map[(int)(player_y + player_vy) * map_width + (int)(player_x + player_vx)] == 0) {
			player_vx *= 1.0 / player_drag;
			player_vy *= 1.0 / player_drag;
			player_x += player_vx;
			player_y += player_vy;
		} else {
			player_vx = 0;
			player_vy = 0;
		}

		// Other input
		if (window_is_key_down(window, INPUT_KEY_Q)) renderer_set_fov(renderer, fov -= 0.01);
		if (window_is_key_down(window, INPUT_KEY_E)) renderer_set_fov(renderer, fov += 0.01);
		if (window_is_key_down(window, INPUT_KEY_Z)) renderer_set_pixelation(renderer, pixelation += 1);
		if (window_is_key_down(window, INPUT_KEY_X)) if (pixelation > 1) renderer_set_pixelation(renderer, pixelation -= 1);
		if (window_is_key_down(window, INPUT_KEY_COMMA)) renderer_set_wallheight(renderer, wallheight -= 0.01);
		if (window_is_key_down(window, INPUT_KEY_PERIOD)) renderer_set_wallheight(renderer, wallheight += 0.01);
		if (window_is_key_down(window, INPUT_KEY_ESCAPE)) is_running = false;

		// Check if user tried to close the window
		if (window_should_close(window))
			is_running = false;

		// Render
		//window_set_as_context(window);
		renderer_clear();
		renderer_draw_floorceiling(renderer, map, map_width, map_height, player_x, player_y, player_r);
		renderer_draw_walls(renderer, map, map_width, map_height, player_x, player_y, player_r);
		window_render(window);
	}

	// Cleanup
	renderer_destroy(renderer);
	window_destroy(window);
}
