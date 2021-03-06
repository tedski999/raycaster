#ifndef RAYCASTER_LIGHT_H
#define RAYCASTER_LIGHT_H

struct raycaster_light;

struct raycaster_light *rc_light_create(int x, int y, unsigned char r, unsigned char g, unsigned char b, int range, double falloff);
void rc_light_set_position(struct raycaster_light *light, int x, int y);
void rc_light_get_position(const struct raycaster_light *const light, int *x, int *y);
void rc_light_set_color(struct raycaster_light *light, unsigned char r, unsigned char g, unsigned char b);
void rc_light_get_color(const struct raycaster_light *const light, unsigned char *r, unsigned char *g, unsigned char *b);
void rc_light_set_lighting(struct raycaster_light *light, int range, double falloff);
void rc_light_get_lighting(const struct raycaster_light *const light, int *range, double *falloff);
void rc_light_destroy(struct raycaster_light *light);

#endif
