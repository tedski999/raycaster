#include "texture.h"
#include "logging.h"
#include "error.h"
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// TODO: replace stb_image

struct rc_texture {
	unsigned char *data;
	int width, height;
};

struct rc_texture *rc_texture_load(const char *filename) {
	rc_log(RC_LOG_VERBOSE, "Loading texture '%s'...", filename);
	struct rc_texture *texture = malloc(sizeof *texture);
	RC_ASSERT(texture);
	texture->data = stbi_load(filename, &texture->width, &texture->height, NULL, 4);
	RC_ASSERT(texture->data);
	return texture;
}

void rc_texture_get_dimensions(const struct rc_texture *texture, int *width, int *height) {
	*width = texture->width;
	*height = texture->height;
}

void rc_texture_get_pixel(const struct rc_texture *texture, int x, int y, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a) {
	const int index = 4 * (y * texture->width + x);
	*r = texture->data[index + 0];
	*g = texture->data[index + 1];
	*b = texture->data[index + 2];
	*a = texture->data[index + 3];
}

void rc_texture_unload(struct rc_texture *texture) {
	rc_log(RC_LOG_VERBOSE, "Unloading texture...");
	stbi_image_free(texture->data);
	free(texture);
}
