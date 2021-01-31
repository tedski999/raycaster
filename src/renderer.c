#include "renderer.h"
#include "error.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <glad/glad.h>

#define FUNKY_LIGHTING 1

struct raycaster_renderer {
	struct raycaster_window *window;
	double aspect, fov, wall_height;
	struct raycaster_texture **wall_textures;
	int num_columns, num_rows;
	unsigned vao, vbo, ibo;
	unsigned tex, double_pbo[2], shader;
	int current_pbo;
};

struct ray_result {
	double distance, latitude;
	int direction, wall;
};

static struct ray_result rc_renderer_internal_raycast(struct raycaster_map *map, double x, double y, double a);
static void rc_renderer_internal_initialize_opengl(struct raycaster_renderer *renderer);
static void rc_renderer_internal_resize_opengl_buffers(struct raycaster_renderer *renderer);
static unsigned rc_renderer_internal_create_shader(const char *const filepath, GLenum shader_type);
static unsigned rc_renderer_internal_create_shader_program(unsigned shaders[], int count);
#ifdef RC_DEBUG
static void rc_renderer_internal_opengl_message_callback(GLenum source, GLenum type, unsigned id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
#endif

struct raycaster_renderer *rc_renderer_create(struct raycaster_window *window, double aspect, int resolution, double fov, double wall_height, struct raycaster_texture **wall_textures) {
	struct raycaster_renderer *renderer = malloc(sizeof *renderer);
	RC_ASSERT(renderer, "raycaster_renderer memory allocation");
	*renderer = (struct raycaster_renderer) { window, aspect };
	rc_renderer_internal_initialize_opengl(renderer);
	rc_renderer_set_fov(renderer, fov);
	rc_renderer_set_wall_height(renderer, wall_height);
	rc_renderer_set_wall_textures(renderer, wall_textures);
	rc_renderer_set_resolution(renderer, resolution);
	rc_window_set_renderer(window, renderer);
	return renderer;
}

void rc_renderer_set_dimensions(struct raycaster_renderer *renderer, int width, int height) {
	double xratio = renderer->aspect * height / width;
	double yratio = 1 / xratio;
	if (xratio > 1) xratio = 1;
	if (yratio > 1) yratio = 1;

	double x = width / 2.0 * (1 - xratio);
	double y = height / 2.0 * (1 - yratio);
	double w = width * xratio;
	double h = height * yratio;

	glViewport(x, y, w, h);
}

void rc_renderer_set_fov(struct raycaster_renderer *renderer, double fov) {
	renderer->fov = fov;
}

void rc_renderer_set_resolution(struct raycaster_renderer *renderer, int resolution) {
	renderer->num_columns = renderer->aspect * resolution;
	renderer->num_rows = resolution;
	rc_renderer_internal_resize_opengl_buffers(renderer);
}

void rc_renderer_set_wall_height(struct raycaster_renderer *renderer, double wall_height) {
	renderer->wall_height = wall_height;
}

void rc_renderer_set_wall_textures(struct raycaster_renderer *renderer, struct raycaster_texture **wall_textures) {
	renderer->wall_textures = wall_textures;
}

void rc_renderer_draw(struct raycaster_renderer *renderer, struct raycaster_map *map, double x, double y, double r) {
	glClear(GL_COLOR_BUFFER_BIT);

	// Render with the current PBO
	glUseProgram(renderer->shader);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, renderer->double_pbo[renderer->current_pbo]);
    glBindTexture(GL_TEXTURE_2D, renderer->tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, renderer->num_columns, renderer->num_rows, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindVertexArray(renderer->vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

	// Flip PBOs and draw to the new current PBO
	renderer->current_pbo ^= 1;
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, renderer->double_pbo[renderer->current_pbo]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, 4 * sizeof (unsigned char) * renderer->num_columns * renderer->num_rows, NULL, GL_STREAM_DRAW);
	unsigned char *pixels = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

	int map_width, map_height;
	rc_map_get_size(map, &map_width, &map_height);

	// Draw floor and ceiling
	double ray_left_rx = cos(r) + sin(r) * renderer->fov, ray_left_ry = sin(r) - cos(r) * renderer->fov;
	double ray_right_rx = cos(r) - sin(r) * renderer->fov, ray_right_ry = sin(r) + cos(r) * renderer->fov;
	for (int row = 0; row < renderer->num_rows / 2; row++) {
		double row_dist = renderer->wall_height * renderer->num_rows / (renderer->num_rows - 2.0 * row);
		row_dist *= 1.0 / renderer->fov;

		for (int column = 0; column < renderer->num_columns; column++) {

			// Find the tile the ray is in and the position within the tile
			double angle = (double)column / renderer->num_columns;
			double ray_x = x + row_dist * (ray_left_rx + angle * (ray_right_rx - ray_left_rx));
			double ray_y = y + row_dist * (ray_left_ry + angle * (ray_right_ry - ray_left_ry));
			int tile_x = (int)ray_x;
			int tile_y = (int)ray_y;
			double texture_x = ray_x - tile_x;
			double texture_y = ray_y - tile_y;

			// Don't draw tiles outside the map
			if (tile_x < 0 || tile_x >= map_width || tile_y < 0 || tile_y >= map_height)
				continue;

			unsigned char r, g, b, a;
			struct raycaster_texture *tex;
			int tex_width, tex_height;
			int pixels_index;

			// Find color of floor at this point
			tex = renderer->wall_textures[rc_map_get_floor(map, tile_x, tile_y)];
			rc_texture_get_dimensions(tex, &tex_width, &tex_height);
			rc_texture_get_pixel(
				tex,
				(int)(tex_width * texture_x) & (tex_width - 1),
				(int)(tex_height * texture_y) & (tex_height - 1),
				&r, &g, &b, &a);
#if FUNKY_LIGHTING
			r *= 1 - row * 2.0 / renderer->num_rows;
			g *= 1 - row * 2.0 / renderer->num_rows;
			b *= 1 - row * 2.0 / renderer->num_rows;
#endif

			// Draw floor
			pixels_index = 4 * (row * renderer->num_columns + column);
			pixels[pixels_index + 0] = r;
			pixels[pixels_index + 1] = g;
			pixels[pixels_index + 2] = b;
			pixels[pixels_index + 3] = 0xff;

			// Find color of ceiling at this point
			tex = renderer->wall_textures[rc_map_get_ceiling(map, tile_x, tile_y)];
			rc_texture_get_dimensions(tex, &tex_width, &tex_height);
			rc_texture_get_pixel(
				tex,
				(int)(tex_width * texture_x) & (tex_width - 1),
				(int)(tex_height * texture_y) & (tex_height - 1),
				&r, &g, &b, &a);
#if FUNKY_LIGHTING
			r *= 1 - row * 2.0 / renderer->num_rows;
			g *= 1 - row * 2.0 / renderer->num_rows;
			b *= 1 - row * 2.0 / renderer->num_rows;
#endif

			// Draw ceiling
			pixels_index = 4 * ((renderer->num_rows - row - 1) * renderer->num_columns + column);
			pixels[pixels_index + 0] = r;
			pixels[pixels_index + 1] = g;
			pixels[pixels_index + 2] = b;
			pixels[pixels_index + 3] = 0xff;
		}
	}

	// Draw walls
	for (int column = 0; column < renderer->num_columns; column++) {

		// Find distance from nearest wall to camera plane
		double ray_offset = ((2.0 * column / renderer->num_columns) - 1) * renderer->fov;
		double ray_rx = cos(r) - sin(r) * ray_offset;
		double ray_ry = sin(r) + cos(r) * ray_offset;
		struct ray_result hit = rc_renderer_internal_raycast(map, x, y, atan2(ray_ry, ray_rx));
		hit.distance *= 1 / sqrt(ray_rx * ray_rx + ray_ry * ray_ry);

		// Don't draw empty walls
		if (hit.wall == -1)
			continue;

		// Wall column
		double wall_length = renderer->wall_height / (hit.distance * renderer->fov);
		int line_length = renderer->num_rows * wall_length;
		int rows_skipped = (line_length - renderer->num_rows) / 2; // pixels of the wall below the screen
		if (rows_skipped < 0)
			rows_skipped = 0;
		int starting_row = (renderer->num_rows - line_length) / 2 + rows_skipped;
		int pixels_index = starting_row * renderer->num_columns + column;

		// Wall texture
		int tex_width, tex_height;
		struct raycaster_texture *tex = renderer->wall_textures[hit.wall];
		rc_texture_get_dimensions(tex, &tex_width, &tex_height);
		double texels_per_row = (double) tex_height / line_length;
		double tex_x = hit.latitude * tex_width, tex_y = tex_height - rows_skipped * texels_per_row - 1;
		if ((hit.direction && ray_rx < 0) || (!hit.direction && ray_ry > 0))
			tex_x = tex_width - tex_x;

		// Iterate over every pixel of the wall within the screen
		for (int i = rows_skipped; i < line_length - rows_skipped; i++) {
			unsigned char r, g, b, a;
			rc_texture_get_pixel(tex, tex_x, tex_y, &r, &g, &b, &a);

#if FUNKY_LIGHTING
			if (wall_length > 1)
				wall_length = 1;
			r *= wall_length;
			g *= wall_length;
			b *= wall_length;
#endif

			pixels[4 * pixels_index + 0] = r;
			pixels[4 * pixels_index + 1] = g;
			pixels[4 * pixels_index + 2] = b;
			pixels[4 * pixels_index + 3] = a;
			pixels_index += renderer->num_columns;
			tex_y -= texels_per_row;
		}

		// TODO: fill z-buffer here
	}

	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void rc_renderer_destroy(struct raycaster_renderer *renderer) {
	glDeleteVertexArrays(1, &renderer->vao);
	glDeleteBuffers(1, &renderer->vbo);
	glDeleteBuffers(1, &renderer->ibo);
	glDeleteBuffers(2, renderer->double_pbo);
	glDeleteTextures(1, &renderer->tex);
	glDeleteProgram(renderer->shader);
}

static struct ray_result rc_renderer_internal_raycast(struct raycaster_map *map, double x, double y, double a) {

	// Starting point
	int tx = x, ty = y;
	double rx = x - tx, ry = y - ty;

	// Direction vector
	double ax = cos(a), ay = sin(a);
	double dx = fabs(1 / ax), dy = fabs(1 / ay);

	// Flip step vector for negative directions
	int sx = -1, sy = -1;
	if (ax >= 0) { rx = 1 - rx; sx = 1; }
	if (ay >= 0) { ry = 1 - ry; sy = 1; }
	rx *= dx; ry *= dy;

	// Get bounds of the map
	int map_width, map_height;
	rc_map_get_size(map, &map_width, &map_height);

	// Euclidean distance to the nearest wall
	struct ray_result hit = { 0, 0, 0, -1 };
	while ((hit.wall = rc_map_get_wall(map, tx, ty)) == -1) {
		if (rx < ry) {
			hit.distance = rx;
			hit.direction = 1;
			rx += dx;
			tx += sx;
		} else {
			hit.distance = ry;
			hit.direction = 0;
			ry += dy;
			ty += sy;
		}

		// If ray is now out of bounds, abort
		if (tx < 0 || tx >= map_width || ty < 0 || ty >= map_height)
			break;
	}

	// Calculate hit position on the wall surface
	if (hit.direction)
		hit.latitude = y + ay * hit.distance;
	else
		hit.latitude = x + ax * hit.distance;
	hit.latitude -= (int)hit.latitude;

	return hit;
}

static void rc_renderer_internal_initialize_opengl(struct raycaster_renderer *renderer) {

	// Load OpenGL functions for the current context
	if (renderer->window)
		gladLoadGLLoader(rc_window_get_load_proc(renderer->window));
	else
		gladLoadGL();
	if (!GLAD_GL_VERSION_3_2) {
		fprintf(stderr, "Your system doesn't support OpenGL >= 3.2!\n");
		exit(1);
	}
#ifdef RC_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(rc_renderer_internal_opengl_message_callback, 0);
#endif
	printf("%s - %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	// A quad which fills the viewport - Will be the canvas for our software rendered texture
	unsigned quad_indices[] = { 0, 1, 3, 1, 2, 3 };
	double quad_vertices[] = {
		 1.0,  1.0, 1.0, 1.0,
		 1.0, -1.0, 1.0, 0.0,
		-1.0, -1.0, 0.0, 0.0,
		-1.0,  1.0, 0.0, 1.0
	};

	// Generate the OpenGL buffers for a quad
	// Create the quad VAO
	glGenVertexArrays(1, &renderer->vao);
	glBindVertexArray(renderer->vao);
	// Create the quad VBO
	glGenBuffers(1, &renderer->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof quad_vertices, quad_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 4 * sizeof (double), (void *) 0);
	glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, 4 * sizeof (double), (void *) (2 * sizeof (double)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	// Create the quad IBO
	glGenBuffers(1, &renderer->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof quad_indices, quad_indices, GL_STATIC_DRAW);
	// Done - Unbind buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Generate the OpenGL buffers for mapping a texture onto the quad with double-buffered PBOs
	glGenBuffers(2, renderer->double_pbo);
	glGenTextures(1, &renderer->tex);
    glBindTexture(GL_TEXTURE_2D, renderer->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

	// Create the shader to be used
	unsigned shaders[2];
	shaders[0] = rc_renderer_internal_create_shader("res/shaders/base.vert", GL_VERTEX_SHADER);
	shaders[1] = rc_renderer_internal_create_shader("res/shaders/base.frag", GL_FRAGMENT_SHADER);
	renderer->shader = rc_renderer_internal_create_shader_program(shaders, 2);
	glDeleteShader(shaders[0]);
	glDeleteShader(shaders[1]);
}

static void rc_renderer_internal_resize_opengl_buffers(struct raycaster_renderer *renderer) {

	// Allocate the first PBO - The second one is allocated during the next render
	// Zero out the new PBO so we don't display garbage for the next frame
	renderer->current_pbo = 0;
	unsigned char blank_frame[4 * renderer->num_columns * renderer->num_rows];
	for (int i = 0; i < 4 * renderer->num_columns * renderer->num_rows; i++)
		blank_frame[i] = 0;
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, renderer->double_pbo[renderer->current_pbo]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, 4 * sizeof (unsigned char) * renderer->num_columns * renderer->num_rows, blank_frame, GL_STREAM_DRAW);

	// Allocate the texture object buffer - Also transfer the new PBO into it
	glBindTexture(GL_TEXTURE_2D, renderer->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderer->num_columns, renderer->num_rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	// Done - Unbind buffers
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

static unsigned rc_renderer_internal_create_shader(const char *const filepath, GLenum shader_type) {
	FILE *file = fopen(filepath, "r");
	if (!file) {
		fprintf(stderr, "Could not read from '%s'!\n", filepath);
		exit(1);
	}

	fseek(file , 0, SEEK_END);
	int file_size = ftell(file);
	rewind(file);

	char *file_buffer = malloc(file_size + 1);
	if (!file_buffer)
		exit(-1);
	if (fread(file_buffer, file_size, 1, file) != 1) {
		fclose(file);
		free(file_buffer);
		fprintf(stderr, "Error while reading '%s'!\n", filepath);
		exit(1);
	}
	fclose(file);
	file_buffer[file_size] = '\0';

	const char *const source = file_buffer;
	unsigned shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	free(file_buffer);

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char desc[512];
		glGetShaderInfoLog(shader, 512, NULL, desc);
		fprintf(stderr, "Error while compiling '%s':\n%s\n", filepath, desc);
		exit(1);
	}

	return shader;
}

static unsigned rc_renderer_internal_create_shader_program(unsigned shaders[], int count) {
	unsigned program = glCreateProgram();
	for (int i = 0; i < count; i++)
		glAttachShader(program, shaders[i]);
	glLinkProgram(program);
	return program;
}

#ifdef RC_DEBUG
static void rc_renderer_internal_opengl_message_callback(GLenum source, GLenum type, unsigned id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	const char *type_str = "UNKN";
	switch (type) {
		case GL_DEBUG_TYPE_ERROR:
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			type_str = "ERRR";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			type_str = "DEPR";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE:
			type_str = "PERF";
			break;
		case GL_DEBUG_TYPE_PORTABILITY:
			type_str = "PORT";
			break;
		case GL_DEBUG_TYPE_MARKER:
		case GL_DEBUG_TYPE_POP_GROUP:
		case GL_DEBUG_TYPE_PUSH_GROUP:
		case GL_DEBUG_TYPE_OTHER:
			type_str = "DBUG";
			break;
	}

	printf("OpenGL (%s): %s\n", type_str, message);
}
#endif
