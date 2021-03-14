#include "platform.h"
#include "logging.h"
#include "window.h"
#include "renderer.h"
#include "texture.h"
#include "entity.h"
#include "player.h"
#include "input.h"
#include "map.h"
#include "light.h"
#include "timer.h"
#include <stdlib.h>
#include <stdbool.h>

// Funky wandering barrel update function
#include <math.h>
void rc_barrel_update(struct rc_entity *barrel, struct rc_map *map) {
	double x, y, z, r;
	rc_entity_get_transform(barrel, &x, &y, &z, &r);

	// Walk forward, turn right if there's a wall 0.5 units in-front
	if (rc_map_get_wall(map, x + cos(r) * 0.5, y + sin(r) * 0.5) != -1)
		r += DEG2RAD(90);
	x += cos(r) * 0.025;
	y += sin(r) * 0.025;

	rc_entity_set_transform(barrel, x, y, z, r);
}

int main(const int argc, const char **argv) {
	rc_log_init();

	// Window config
	const int window_width = 640, window_height = 480;
	const double window_aspect = 16.0 / 9.0;
	const bool window_is_resizable = true, window_is_cursor_disabled = false;

	// Debug variables
	int tps = 60;                 // ticks per second
	int resolution = 200;         // number of vertical pixels
	double fov = DEG2RAD(60);     // field of view
	bool is_vsync_enabled = true; // if glfw will wait for vsync

	// Load textures
	rc_log(RC_LOG_INFO, "Loading textures...");
	const int wall_textures_count = 8;
	struct rc_texture *wall_textures[8] = {
		rc_texture_load("res/textures/wood.png"),
		rc_texture_load("res/textures/greystone.png"),
		rc_texture_load("res/textures/mossy.png"),
		rc_texture_load("res/textures/bluestone.png"),
		rc_texture_load("res/textures/purplestone.png"),
		rc_texture_load("res/textures/colorstone.png"),
		rc_texture_load("res/textures/redbrick.png"),
		rc_texture_load("res/textures/eagle.png")
	};
	struct rc_texture *light_texture = rc_texture_load("res/textures/barrel.png");

	// Debug map
	rc_log(RC_LOG_INFO, "Initializing game world...");
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
	// TODO: entities should be able to create and modify their own lights
	const int map_lights_count = 3;
	struct rc_light *map_lights[3] = {
		rc_light_create(1,  1, 0xff, 0x00, 0x00, 10, 5.0),
		rc_light_create(10, 7, 0x00, 0x60, 0xff, 10, 5.0),
		rc_light_create(17, 7, 0x40, 0x40, 0x40, 10, 5.0)
	};
	// TODO: a better entities data structure, optimized for calculating distance from the player
	// entities should also be modifiable from anywhere? e.g a player should be able to make a projectile
	const int entities_count = 8;
	struct rc_entity *entities[8] = {
		rc_entity_create(2.5,  2.5, 0.5, 0.0, NULL, rc_player_init, rc_player_update, rc_player_destroy),
		rc_entity_create(10.5, 7.5, 0.5, 0.0, light_texture, NULL, rc_barrel_update, NULL),
		rc_entity_create(2.5,  2.5, 0.5, 0.0, light_texture, NULL, rc_barrel_update, NULL),
		rc_entity_create(18.5, 2.5, 0.5, 0.0, light_texture, NULL, rc_barrel_update, NULL),
		rc_entity_create(7.5,  5.5, 0.5, 0.0, light_texture, NULL, rc_barrel_update, NULL),
		rc_entity_create(18.5, 5.5, 0.5, 0.0, light_texture, NULL, rc_barrel_update, NULL),
		rc_entity_create(2.5,  3.5, 0.5, 0.0, light_texture, NULL, rc_barrel_update, NULL),
		rc_entity_create(12.5, 3.5, 0.5, 0.0, light_texture, NULL, rc_barrel_update, NULL)
	};
	struct rc_entity *player = entities[0];

	// Create the window, renderer and map
	struct rc_window *window = rc_window_create("raycaster", window_width, window_height, window_is_resizable, window_is_cursor_disabled, is_vsync_enabled);
	struct rc_renderer *renderer = rc_renderer_create(window, window_aspect, resolution, fov, wall_textures);
	struct rc_map *map = rc_map_create(map_width, map_height, map_floor, map_walls, map_ceiling);

	// Main game loop
	bool is_running = true;
	double accumulated_time = 0;
	struct rc_timer *timer = rc_timer_create();
	rc_log(RC_LOG_NOTEWORTHY, "Entering main game loop...");
	while (is_running) {

		// Find deltatime
		const double dt = rc_timer_reset(timer);
		accumulated_time += dt;

		// Update 60 times a second
		while (is_running && accumulated_time >= 1.0 / tps) {
			accumulated_time -= 1.0 / tps;

			// Update
			rc_window_update(window);
			for (int i = 0; i < entities_count; i++)
				rc_entity_update(entities[i], map);
			rc_map_generate_lighting(map, 0x10, 0x10, 0x10, map_lights, map_lights_count);
			if (rc_window_should_close(window))
				is_running = false;

			// Debug input - TODO: write these as text to the screen
			if (rc_input_is_key_down(RC_INPUT_KEY_COMMA))     rc_renderer_set_fov(renderer, fov -= 0.01);
			if (rc_input_is_key_down(RC_INPUT_KEY_PERIOD))    rc_renderer_set_fov(renderer, fov += 0.01);
			if (rc_input_is_key_pressed(RC_INPUT_KEY_MINUS))  if (resolution > 1) rc_renderer_set_resolution(renderer, resolution -= 2);
			if (rc_input_is_key_pressed(RC_INPUT_KEY_EQUALS)) rc_renderer_set_resolution(renderer, resolution += 2);
			if (rc_input_is_key_pressed(RC_INPUT_KEY_V))      rc_window_set_vsync_enabled(window, is_vsync_enabled = !is_vsync_enabled);
			if (rc_input_is_key_pressed(RC_INPUT_KEY_ESCAPE)) is_running = false;

			rc_input_update();
		}

		// Render asap
		rc_window_set_as_context(window);
		rc_renderer_draw(renderer, map, entities, entities_count, player);
		rc_window_render(window);
	}

	// Cleanup
	rc_log(RC_LOG_NOTEWORTHY, "Cleaning up...");
	rc_timer_destroy(timer);
	rc_map_destroy(map);
	rc_renderer_destroy(renderer);
	rc_window_destroy(window);
	for (int i = 0; i < map_lights_count; i++)
		rc_light_destroy(map_lights[i]);
	for (int i = 0; i < entities_count; i++)
		rc_entity_destroy(entities[i]);
	for (int i = 0; i < wall_textures_count; i++)
		rc_texture_unload(wall_textures[i]);
	rc_texture_unload(light_texture);
	rc_log_cleanup();
}
