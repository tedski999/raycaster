#ifndef RC_TEXTURE_H
#define RC_TEXTURE_H

struct rc_texture;

struct rc_texture *rc_texture_load(const char *filename);
void rc_texture_get_dimensions(const struct rc_texture *texture, int *width, int *height);
void rc_texture_get_pixel(const struct rc_texture *texture, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a);
void rc_texture_unload(struct rc_texture *texture);

#endif
