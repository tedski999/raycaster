#include "player.h"
#include "error.h"
#include <stdlib.h>
#include <math.h>

// TODO: configurable
#define player_accel 0.01
#define player_drag 1.2
#define player_turn_speed 0.04
#define mouse_sensitivity 0.01

struct raycaster_player {
	struct raycaster_window *window;
	struct raycaster_map *map;
	double position_x, position_y, rotation;
	double velocity_x, velocity_y;
	double old_mouse_x, old_mouse_y;
};

struct raycaster_player *rc_player_create(struct raycaster_window *window, struct raycaster_map *map, double position_x, double position_y, double rotation) {
	struct raycaster_player *player = malloc(sizeof *player);
	RC_ASSERT(player, "raycaster_player memory allocation");
	*player = (struct raycaster_player) { window, map, position_x, position_y, rotation };
	rc_window_get_mouse_position(window, &player->old_mouse_x, &player->old_mouse_y);
	return player;
}

void rc_player_update(struct raycaster_player *player) {

	// Mouse velocity - TODO: this should be moved to an input module
	double mouse_x, mouse_y, mouse_vx, mouse_vy;
	rc_window_get_mouse_position(player->window, &mouse_x, &mouse_y);
	mouse_vx = mouse_x - player->old_mouse_x;
	mouse_vy = mouse_y - player->old_mouse_y;
	player->old_mouse_x = mouse_x;
	player->old_mouse_y = mouse_y;

	// Player rotation input
	player->rotation += mouse_vx * mouse_sensitivity;
	if (rc_window_is_key_down(player->window, INPUT_KEY_RIGHT)) player->rotation += player_turn_speed;
	if (rc_window_is_key_down(player->window, INPUT_KEY_LEFT)) player->rotation -= player_turn_speed;

	// Player movement input
	if (rc_window_is_key_down(player->window, INPUT_KEY_W)) { player->velocity_x += cos(player->rotation) * player_accel; player->velocity_y += sin(player->rotation) * player_accel; }
	if (rc_window_is_key_down(player->window, INPUT_KEY_S)) { player->velocity_x -= cos(player->rotation) * player_accel; player->velocity_y -= sin(player->rotation) * player_accel; }
	if (rc_window_is_key_down(player->window, INPUT_KEY_D)) { player->velocity_x -= sin(player->rotation) * player_accel; player->velocity_y += cos(player->rotation) * player_accel; }
	if (rc_window_is_key_down(player->window, INPUT_KEY_A)) { player->velocity_x += sin(player->rotation) * player_accel; player->velocity_y -= cos(player->rotation) * player_accel; }

	// Check for collision before applying velocity
	if (rc_map_get_wall(player->map, (int)(player->position_x + player->velocity_x), (int)(player->position_y + player->velocity_y)) == 0) {
		player->velocity_x *= 1.0 / player_drag;
		player->velocity_y *= 1.0 / player_drag;
		player->position_x += player->velocity_x;
		player->position_y += player->velocity_y;
	} else {
		player->velocity_x = 0;
		player->velocity_y = 0;
	}
}

void rc_player_get_transform(struct raycaster_player *player, double *position_x, double *position_y, double *rotation) {
	*position_x = player->position_x;
	*position_y = player->position_y;
	*rotation = player->rotation;
}

void rc_player_destroy(struct raycaster_player *player) {
	free(player);
}
