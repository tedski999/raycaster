#include "player.h"
#include "input.h"
#include "error.h"
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

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

struct raycaster_player_data {
	double vel_x, vel_y;
	int movement_ticks;
};

void rc_player_init(struct raycaster_entity *player) {
	struct raycaster_player_data *player_data = calloc(1, sizeof *player_data);
	RC_ASSERT(player_data, "raycaster_player_data memory allocation");
	rc_entity_set_data_pointer(player, player_data);
}

void rc_player_update(struct raycaster_entity *player, struct raycaster_map *map) {

	double player_x, player_y, player_z, player_r;
	rc_entity_get_transform(player, &player_x, &player_y, &player_z, &player_r);
	struct raycaster_player_data *player_data = rc_entity_get_data_pointer(player);

	// Crawling
	bool is_crawling = rc_input_is_key_down(INPUT_KEY_SHIFT);
	double target_height = (is_crawling) ? player_crawl_height : player_normal_height;
	player_z = player_z * (1 - player_height_speed) + target_height * player_height_speed;

	// Player rotation input
	double mouse_vx, mouse_vy;
	rc_input_get_mouse_velocity(&mouse_vx, &mouse_vy);
	player_r += mouse_vx;
	if (rc_input_is_key_down(INPUT_KEY_RIGHT)) player_r += player_turn_speed;
	if (rc_input_is_key_down(INPUT_KEY_LEFT)) player_r -= player_turn_speed;

	// Player movement input
	double accel = (is_crawling) ? player_crawl_accel : player_normal_accel;
	double max_speed = (is_crawling) ? player_crawl_speed : player_normal_speed;
	double target_vel_x = 0, target_vel_y = 0;
	if (rc_input_is_key_down(INPUT_KEY_W)) { target_vel_x += cos(player_r) * max_speed; target_vel_y += sin(player_r) * max_speed; }
	if (rc_input_is_key_down(INPUT_KEY_S)) { target_vel_x -= cos(player_r) * max_speed; target_vel_y -= sin(player_r) * max_speed; }
	if (rc_input_is_key_down(INPUT_KEY_D)) { target_vel_x -= sin(player_r) * max_speed; target_vel_y += cos(player_r) * max_speed; }
	if (rc_input_is_key_down(INPUT_KEY_A)) { target_vel_x += sin(player_r) * max_speed; target_vel_y -= cos(player_r) * max_speed; }
	player_data->vel_x = player_data->vel_x * (1 - accel) + target_vel_x * accel;
	player_data->vel_y = player_data->vel_y * (1 - accel) + target_vel_y * accel;

	// Check for collision before applying velocity
	if (rc_map_get_wall(map, floor(player_x + player_data->vel_x), player_y) == -1)
		player_x += player_data->vel_x;
	if (rc_map_get_wall(map, player_x, floor(player_y + player_data->vel_y)) == -1)
		player_y += player_data->vel_y;

	// Head bobbing
	player_data->movement_ticks++;
	double vel_mag = sqrt(player_data->vel_x * player_data->vel_x + player_data->vel_y * player_data->vel_y);
	if (vel_mag < player_min_speed)
		player_data->movement_ticks = 0;
	double freq = (is_crawling) ? player_crawl_bobbing_freq : player_normal_bobbing_freq;
	double mag = (is_crawling) ? player_crawl_bobbing_mag : player_normal_bobbing_mag;
	player_z += sin(player_data->movement_ticks * freq) * mag * vel_mag / max_speed;

	rc_entity_set_transform(player, player_x, player_y, player_z, player_r);
}

void rc_player_destroy(struct raycaster_entity *player) {
	free(rc_entity_get_data_pointer(player));
}
