#ifndef RAYCASTER_TEXTURE_H
#define RAYCASTER_TEXTURE_H

struct raycaster_texture;

struct raycaster_texture *rc_texture_load(const char *const filename);
void rc_texture_get_dimensions(struct raycaster_texture *texture, int *width, int *height);
void rc_texture_get_pixel(struct raycaster_texture *texture, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a);
void rc_texture_unload(struct raycaster_texture *texture);

#endif
