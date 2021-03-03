#include "texture.h"
#include "error.h"
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// TODO: replace stb_image

struct raycaster_texture {
	unsigned char *data;
	int width, height;
};

struct raycaster_texture *rc_texture_load(const char *const filename) {
	struct raycaster_texture *texture = malloc(sizeof *texture);
	RC_ASSERT(texture, "raycaster_texture memory allocation");
	texture->data = stbi_load(filename, &texture->width, &texture->height, NULL, 4);
	RC_ASSERT(texture->data, "raycaster_texture stbi_load");
	return texture;
}

void rc_texture_get_dimensions(struct raycaster_texture *texture, int *width, int *height) {
	*width = texture->width;
	*height = texture->height;
}

void rc_texture_get_pixel(struct raycaster_texture *texture, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a) {
	int index = 4 * (y * texture->width + x);
	*r = texture->data[index + 0];
	*g = texture->data[index + 1];
	*b = texture->data[index + 2];
	*a = texture->data[index + 3];
}

void rc_texture_unload(struct raycaster_texture *texture) {
	stbi_image_free(texture->data);
	free(texture);
}
