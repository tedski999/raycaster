#ifndef RC_MAP_H
#define RC_MAP_H

struct rc_light;
struct rc_map;

struct rc_map *rc_map_create(int map_width, int map_height, const int *floor, const int *walls, const int *ceiling);
void rc_map_get_size(const struct rc_map *map, int *width, int *height);
int rc_map_get_floor(const struct rc_map *map, int x, int y);
int rc_map_get_wall(const struct rc_map *map, int x, int y);
int rc_map_get_ceiling(const struct rc_map *map, int x, int y);
void rc_map_generate_lighting(const struct rc_map *map, unsigned char ambient_r, unsigned char ambient_g, unsigned char ambient_b, struct rc_light **lights, int lights_count);
void rc_map_get_lighting(const struct rc_map *map, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b);
void rc_map_destroy(struct rc_map *map);

#endif
