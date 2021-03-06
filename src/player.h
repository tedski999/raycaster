#ifndef RAYCASTER_PLAYER_H
#define RAYCASTER_PLAYER_H

struct raycaster_entity;
struct raycaster_map;

void rc_player_init(struct raycaster_entity *player);
void rc_player_update(struct raycaster_entity *player, struct raycaster_map *map);
void rc_player_destroy(struct raycaster_entity *player);

#endif
