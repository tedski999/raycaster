#ifndef RC_PLAYER_H
#define RC_PLAYER_H

struct rc_entity;
struct rc_map;

void rc_player_init(struct rc_entity *player);
void rc_player_update(struct rc_entity *player, struct rc_map *map);
void rc_player_destroy(struct rc_entity *player);

#endif
