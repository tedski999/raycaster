#ifndef RAYCASTER_PLAYER_H
#define RAYCASTER_PLAYER_H

#include "window.h"
#include "map.h"

struct raycaster_player;

struct raycaster_player *rc_player_create(struct raycaster_window *window, struct raycaster_map *map, double position_x, double position_y, double rotation);
void rc_player_update(struct raycaster_player *player);
void rc_player_get_transform(struct raycaster_player *player, double *position_x, double *position_y, double *rotation);
void rc_player_destroy(struct raycaster_player *player);

#endif
