#include "player.h"
#include "error.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define mouse_sensitivity 0.01

#define player_normal_height 0.5
#define player_crawl_height 0.1
#define player_height_speed 0.25

#define player_turn_speed 0.04
#define player_normal_speed 0.05
#define player_normal_accel 0.15
#define player_crawl_speed 0.015
#define player_crawl_accel 0.25
#define player_min_speed 0.01

#define player_normal_bobbing_mag 0.01
#define player_normal_bobbing_freq 0.2
#define player_crawl_bobbing_mag 0.005
#define player_crawl_bobbing_freq 0.15

struct raycaster_player {
	struct raycaster_window *window;
	struct raycaster_map *map;
	double position_x, position_y, position_z, rotation;
	double velocity_x, velocity_y;
	double old_mouse_x, old_mouse_y;
	int movement_ticks;
};

struct raycaster_player *rc_player_create(struct raycaster_window *window, struct raycaster_map *map, double position_x, double position_y, double position_z, double rotation) {
	struct raycaster_player *player = malloc(sizeof *player);
	RC_ASSERT(player, "raycaster_player memory allocation");
	*player = (struct raycaster_player) { window, map, position_x, position_y, position_z, rotation };
	rc_window_get_mouse_position(window, &player->old_mouse_x, &player->old_mouse_y);
	return player;
}

void rc_player_update(struct raycaster_player *player) {

	// Mouse velocity - TODO: this should be moved to an input module
	double mouse_x, mouse_y, mouse_vx;
	rc_window_get_mouse_position(player->window, &mouse_x, &mouse_y);
	mouse_vx = mouse_x - player->old_mouse_x;
	player->old_mouse_x = mouse_x;
	player->old_mouse_y = mouse_y;

	// Player rotation input
	player->rotation += mouse_vx * mouse_sensitivity;
	if (rc_window_is_key_down(player->window, INPUT_KEY_RIGHT)) player->rotation += player_turn_speed;
	if (rc_window_is_key_down(player->window, INPUT_KEY_LEFT)) player->rotation -= player_turn_speed;

	// Crawling
	bool is_crawling = rc_window_is_key_down(player->window, INPUT_KEY_TAB);
	double target_height = (is_crawling) ? player_crawl_height : player_normal_height;
	player->position_z = player->position_z * (1 - player_height_speed) + target_height * player_height_speed;

	// Player movement input
	double accel = (is_crawling) ? player_crawl_accel : player_normal_accel;
	double max_speed = (is_crawling) ? player_crawl_speed : player_normal_speed;
	double target_velocity_x = 0, target_velocity_y = 0;
	if (rc_window_is_key_down(player->window, INPUT_KEY_W)) { target_velocity_x += cos(player->rotation) * max_speed; target_velocity_y += sin(player->rotation) * max_speed; }
	if (rc_window_is_key_down(player->window, INPUT_KEY_S)) { target_velocity_x -= cos(player->rotation) * max_speed; target_velocity_y -= sin(player->rotation) * max_speed; }
	if (rc_window_is_key_down(player->window, INPUT_KEY_D)) { target_velocity_x -= sin(player->rotation) * max_speed; target_velocity_y += cos(player->rotation) * max_speed; }
	if (rc_window_is_key_down(player->window, INPUT_KEY_A)) { target_velocity_x += sin(player->rotation) * max_speed; target_velocity_y -= cos(player->rotation) * max_speed; }
	player->velocity_x = player->velocity_x * (1 - accel) + target_velocity_x * accel;
	player->velocity_y = player->velocity_y * (1 - accel) + target_velocity_y * accel;

	// Check for collision before applying velocity
	if (rc_map_get_wall(player->map, floor(player->position_x + player->velocity_x), player->position_y) == -1)
		player->position_x += player->velocity_x;
	if (rc_map_get_wall(player->map, player->position_x, floor(player->position_y + player->velocity_y)) == -1)
		player->position_y += player->velocity_y;

	// Head bobbing
	player->movement_ticks++;
	double velocity_mag = sqrt(player->velocity_x * player->velocity_x + player->velocity_y * player->velocity_y);
	if (velocity_mag < player_min_speed)
		player->movement_ticks = 0;
	double freq = (is_crawling) ? player_crawl_bobbing_freq : player_normal_bobbing_freq;
	double mag = (is_crawling) ? player_crawl_bobbing_mag : player_normal_bobbing_mag;
	player->position_z += sin(player->movement_ticks * freq) * mag * velocity_mag / max_speed;
}

void rc_player_get_transform(struct raycaster_player *player, double *position_x, double *position_y, double *position_z, double *rotation) {
	*position_x = player->position_x;
	*position_y = player->position_y;
	*position_z = player->position_z;
	*rotation = player->rotation;
}

void rc_player_destroy(struct raycaster_player *player) {
	free(player);
}
