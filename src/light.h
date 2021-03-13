#ifndef RC_LIGHT_H
#define RC_LIGHT_H

struct rc_light;

struct rc_light *rc_light_create(int x, int y, unsigned char r, unsigned char g, unsigned char b, int range, double falloff);
void rc_light_set_position(struct rc_light *light, int x, int y);
void rc_light_get_position(const struct rc_light *light, int *x, int *y);
void rc_light_set_color(struct rc_light *light, unsigned char r, unsigned char g, unsigned char b);
void rc_light_get_color(const struct rc_light *light, unsigned char *r, unsigned char *g, unsigned char *b);
void rc_light_set_lighting(struct rc_light *light, int range, double falloff);
void rc_light_get_lighting(const struct rc_light *light, int *range, double *falloff);
void rc_light_destroy(struct rc_light *light);

#endif
