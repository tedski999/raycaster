#include "light.h"
#include "error.h"
#include <stdlib.h>

struct rc_light {
	int x, y;
	unsigned char r, g, b;
	int range;
	double falloff;
};

struct rc_light *rc_light_create(int x, int y, unsigned char r, unsigned char g, unsigned char b, int range, double falloff) {
	struct rc_light *light = malloc(sizeof *light);
	RC_ASSERT(light);
	*light = (struct rc_light) { x, y, r, g, b, range, falloff };
	return light;
}

void rc_light_set_position(struct rc_light *light, int x, int y) {
	light->x = x;
	light->y = y;
}

void rc_light_get_position(const struct rc_light *light, int *x, int *y) {
	*x = light->x;
	*y = light->y;
}

void rc_light_set_color(struct rc_light *light, unsigned char r, unsigned char g, unsigned char b) {
	light->r = r;
	light->g = g;
	light->b = b;
}

void rc_light_get_color(const struct rc_light *light, unsigned char *r, unsigned char *g, unsigned char *b) {
	*r = light->r;
	*g = light->g;
	*b = light->b;

}

void rc_light_set_lighting(struct rc_light *light, int range, double falloff) {
	light->range = range;
	light->falloff = falloff;
}

void rc_light_get_lighting(const struct rc_light *light, int *range, double *falloff) {
	*range = light->range;
	*falloff = light->falloff;
}

void rc_light_destroy(struct rc_light *light) {
	free(light);
}
