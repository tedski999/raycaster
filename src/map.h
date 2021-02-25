#ifndef RAYCASTER_MAP_H
#define RAYCASTER_MAP_H

struct raycaster_map;

struct raycaster_light {
	int x, y;
	unsigned char r, g, b;
	int range;
	double falloff;
};

struct raycaster_map *rc_map_create(int map_width, int map_height, const int *const floor, const int *const walls, const int *const ceiling);
void rc_map_regenerate_lighting(struct raycaster_map *map, unsigned char ambient_r, unsigned char ambient_g, unsigned char ambient_b, const struct raycaster_light *const lights, int lights_count);
void rc_map_get_size(struct raycaster_map *map, int *width, int *height);
int rc_map_get_floor(struct raycaster_map *map, int x, int y);
int rc_map_get_wall(struct raycaster_map *map, int x, int y);
int rc_map_get_ceiling(struct raycaster_map *map, int x, int y);
void rc_map_get_lighting(struct raycaster_map *map, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b);
void rc_map_destroy(struct raycaster_map *map);

#endif
