#include "window.h"
#include "renderer.h"
#include "entity.h"
#include "player.h"
#include "input.h"
#include "map.h"
#include "timer.h"
#include <stdlib.h>
#include <stdbool.h>

#define DEG2RAD(degree) (degree * 3.14152 / 180)

int main(int argc, char **argv) {

	// Window config
	const int window_width = 640, window_height = 480;
	const double window_aspect = 16.0 / 9.0;
	const bool window_is_resizable = true, window_is_cursor_disabled = false;

	// Debug variables
	int tps = 60;                 // ticks per second
	int resolution = 200;         // number of vertical pixels
	double fov = DEG2RAD(60);     // field of view
	double wall_height = 1.0;     // height of raycast walls multiplier
	bool is_vsync_enabled = true; // if glfw will wait for vsync

	// Debug map
	const int map_width = 20, map_height = 10;
	const int map_floor[20*10] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 4, 4, 4, 0,
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 0, 0, 4, 4, 4, 0,
		0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 4, 4, 4, 0,
		0, 0, 0, 3, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 4, 0, 0,
		0, 0, 0, 3, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	const int map_walls[20*10] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 4, 4, 4, 4,
		1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1, 3, 1, 1, 4,-1,-1,-1, 4,
		1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 4,-1,-1,-1, 4,
		1,-1,-1, 3,-1,-1, 0,-1,-1, 0,-1,-1, 3,-1, 1, 4,-1,-1,-1, 4,
		1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1,-1, 1, 4, 4,-1, 4, 4,
		1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1,-1, 1, 6, 6,-1, 6, 6,
		1,-1,-1, 3,-1,-1, 0,-1,-1, 0,-1,-1, 1,-1, 1, 6,-1,-1,-1, 6,
		1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1, 1,-1,-1,-1,-1,-1,-1, 6,
		1,-1,-1, 1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 6,-1,-1,-1, 6,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6, 6, 7, 6, 6
	};
	const int map_ceiling[20*10] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 4, 4, 4, 0,
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 0, 0, 4, 4, 4, 0,
		0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 4, 4, 4, 0,
		0, 0, 0, 3, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 4, 0, 0,
		0, 0, 0, 3, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 6, 0, 0,
		0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 6, 6, 6, 0,
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 6, 6, 6, 6, 0,
		0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 6, 6, 6, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	const int map_lights_count = 3;
	const struct raycaster_light map_lights[3] = {
		{ 1,  1, 0xff, 0x00, 0x00, 10, 5.0 },
		{ 10, 7, 0x00, 0x60, 0xff, 10, 5.0 },
		{ 17, 7, 0x40, 0x40, 0x40, 10, 5.0 },
	};
	const int entities_count = 1;
	struct raycaster_entity *entities[1] = {
		rc_entity_create(2.5, 2.5, 0.5, 0.0, NULL, rc_player_init, rc_player_update, rc_player_destroy)
	};
	struct raycaster_entity *player = entities[0];

	// Load textures
	const int wall_textures_count = 8;
	struct raycaster_texture *wall_textures[8] = {
		rc_texture_load("res/textures/wood.png"),
		rc_texture_load("res/textures/greystone.png"),
		rc_texture_load("res/textures/mossy.png"),
		rc_texture_load("res/textures/bluestone.png"),
		rc_texture_load("res/textures/purplestone.png"),
		rc_texture_load("res/textures/colorstone.png"),
		rc_texture_load("res/textures/redbrick.png"),
		rc_texture_load("res/textures/eagle.png")
	};

	// Create the window, renderer and map
	struct raycaster_window *window = rc_window_create("raycaster", window_width, window_height, window_is_resizable, window_is_cursor_disabled, is_vsync_enabled);
	struct raycaster_renderer *renderer = rc_renderer_create(window, window_aspect, resolution, fov, wall_height, wall_textures);
	struct raycaster_map *map = rc_map_create(map_width, map_height, map_floor, map_walls, map_ceiling);
	rc_map_regenerate_lighting(map, 0x10, 0x10, 0x10, map_lights, map_lights_count);

	// Main game loop
	bool is_running = true;
	double accumulated_time = 0;
	struct raycaster_timer *timer = rc_timer_create();
	while (is_running) {

		// Find deltatime
		double dt = rc_timer_reset(timer);
		accumulated_time += dt;

		// Update 60 times a second
		while (is_running && accumulated_time >= 1.0 / tps) {
			accumulated_time -= 1.0 / tps;

			// Update
			rc_window_update(window);
			for (int i = 0; i < entities_count; i++)
				rc_entity_update(entities[i], map);
			if (rc_window_should_close(window))
				is_running = false;

			// Debug input - TODO: write these as text to the screen
			if (rc_input_is_key_down(INPUT_KEY_COMMA))         rc_renderer_set_fov(renderer, fov -= 0.01);
			if (rc_input_is_key_down(INPUT_KEY_PERIOD))        rc_renderer_set_fov(renderer, fov += 0.01);
			if (rc_input_is_key_down(INPUT_KEY_LEFT_BRACKET))  rc_renderer_set_wall_height(renderer, wall_height -= 0.01);
			if (rc_input_is_key_down(INPUT_KEY_RIGHT_BRACKET)) rc_renderer_set_wall_height(renderer, wall_height += 0.01);
			if (rc_input_is_key_pressed(INPUT_KEY_MINUS))      if (resolution > 1) rc_renderer_set_resolution(renderer, resolution -= 2);
			if (rc_input_is_key_pressed(INPUT_KEY_EQUALS))     rc_renderer_set_resolution(renderer, resolution += 2);
			if (rc_input_is_key_pressed(INPUT_KEY_V))          rc_window_set_vsync_enabled(window, is_vsync_enabled = !is_vsync_enabled);
			if (rc_input_is_key_pressed(INPUT_KEY_L))          rc_map_regenerate_lighting(map, 0x10, 0x10, 0x10, map_lights, map_lights_count);
			if (rc_input_is_key_pressed(INPUT_KEY_ESCAPE))     is_running = false;

			rc_input_update();
		}

		// Render asap
		double player_x, player_y, player_z, player_r;
		rc_entity_get_transform(player, &player_x, &player_y, &player_z, &player_r);
		rc_window_set_as_context(window);
		rc_renderer_draw(renderer, map, player_x, player_y, player_z, player_r);
		rc_window_render(window);
	}

	// Cleanup
	rc_timer_destroy(timer);
	rc_map_destroy(map);
	rc_renderer_destroy(renderer);
	rc_window_destroy(window);
	for (int i = 0; i < entities_count; i++)
		rc_entity_destroy(entities[i]);
	for (int i = 0; i < wall_textures_count; i++)
		rc_texture_unload(wall_textures[i]);
}
