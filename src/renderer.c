#include "renderer.h"
#include "error.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <glad/glad.h>

struct raycaster_renderer {
	int width, height, pixelation;
	double aspect, fov, wallheight;
	unsigned vao, vbo, ibo;
	unsigned tex, double_pbo[2], shader;
	int current_pbo;
};

struct ray_result {
	double distance;
	int direction;
	int wall;
};

static struct ray_result rc_renderer_internal_raycast(struct raycaster_map *map, double x, double y, double a);
static void rc_renderer_internal_resize(struct raycaster_renderer *renderer);
static unsigned rc_renderer_internal_create_shader(const char *const filepath, GLenum shader_type);
static unsigned rc_renderer_internal_create_shader_program(unsigned shaders[], int count);
#ifdef RC_DEBUG
static void rc_renderer_internal_opengl_message_callback(GLenum source, GLenum type, unsigned id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
#endif

struct raycaster_renderer *rc_renderer_create(int width, int height, double aspect, double fov, int pixelation, double wallheight) {

	struct raycaster_renderer *renderer = malloc(sizeof *renderer);
	RC_ASSERT(renderer, "raycaster_renderer memory allocation");
	*renderer = (struct raycaster_renderer) { width, height, pixelation, aspect, fov, wallheight };

	// Load OpenGL functions for the current context
	gladLoadGL();
	if (!GLAD_GL_VERSION_3_2) {
		fprintf(stderr, "Your system doesn't support OpenGL >= 3.2!\n");
		exit(1);
	}
	printf("%s - %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Configure OpenGL
#ifdef RC_DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(rc_renderer_internal_opengl_message_callback, 0);
#endif

	// Initialize the OpenGL buffers for software renderering
	glGenVertexArrays(1, &renderer->vao);
	glBindVertexArray(renderer->vao);

	// Create the quad VBO
	double quad_vertices[] = {
		 1.0,  1.0, 1.0, 1.0,
		 1.0, -1.0, 1.0, 0.0,
		-1.0, -1.0, 0.0, 0.0,
		-1.0,  1.0, 0.0, 1.0
	};
	glGenBuffers(1, &renderer->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof quad_vertices, quad_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 4 * sizeof (double), (void *) 0);
	glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, 4 * sizeof (double), (void *) (2 * sizeof (double)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Create the quad IBO
	unsigned quad_indices[] = { 0, 1, 3, 1, 2, 3 };
	glGenBuffers(1, &renderer->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof quad_indices, quad_indices, GL_STATIC_DRAW);

	// Create the quad double PBOs and their texture object
	glGenBuffers(2, renderer->double_pbo);
	glGenTextures(1, &renderer->tex);

	// Setup the shader
	unsigned shaders[2];
	shaders[0] = rc_renderer_internal_create_shader("res/shaders/base.vert", GL_VERTEX_SHADER);
	shaders[1] = rc_renderer_internal_create_shader("res/shaders/base.frag", GL_FRAGMENT_SHADER);
	renderer->shader = rc_renderer_internal_create_shader_program(shaders, 2);
	glDeleteShader(shaders[0]);
	glDeleteShader(shaders[1]);

	glBindVertexArray(0);
	rc_renderer_internal_resize(renderer);
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

	renderer->width = w;
	renderer->height = h;
	glViewport(x, y, w, h);
	rc_renderer_internal_resize(renderer);
}

void rc_renderer_set_fov(struct raycaster_renderer *renderer, double fov) {
	renderer->fov = fov;
}

void rc_renderer_set_pixelation(struct raycaster_renderer *renderer, int pixelation) {
	renderer->pixelation = pixelation;
	rc_renderer_internal_resize(renderer);
}

void rc_renderer_set_wallheight(struct raycaster_renderer *renderer, double wallheight) {
	renderer->wallheight = wallheight;
}

void rc_renderer_draw(struct raycaster_renderer *renderer, struct raycaster_map *map, double x, double y, double r) {
	int num_columns = renderer->width / renderer->pixelation;
	int num_rows = renderer->height / renderer->pixelation;

	glClear(GL_COLOR_BUFFER_BIT);

	// Render with the current PBO
	glUseProgram(renderer->shader);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, renderer->double_pbo[renderer->current_pbo]);
    glBindTexture(GL_TEXTURE_2D, renderer->tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, num_columns, num_rows, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindVertexArray(renderer->vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

	// Flip PBOs and draw to the new current PBO
	renderer->current_pbo ^= 1;
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, renderer->double_pbo[renderer->current_pbo]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, 4 * sizeof (unsigned char) * num_columns * num_rows, NULL, GL_STREAM_DRAW);
	unsigned char *pixels = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

	// Draw floor and ceiling
	double ray_left_rx = cos(r) + sin(r) * renderer->fov, ray_left_ry = sin(r) - cos(r) * renderer->fov;
	double ray_right_rx = cos(r) - sin(r) * renderer->fov, ray_right_ry = sin(r) + cos(r) * renderer->fov;
	for (int row = 0; row < num_rows / 2; row++) {
		double row_dist = renderer->wallheight * num_rows / (num_rows - 2.0 * row);
		row_dist *= 1.0 / renderer->fov;

		for (int column = 0; column < num_columns; column++) {
			double angle = (double)column / num_columns;
			double xtile = x + row_dist * (ray_left_rx + angle * (ray_right_rx - ray_left_rx));
			double ytile = y + row_dist * (ray_left_ry + angle * (ray_right_ry - ray_left_ry));

			int checkerboard = ((int)xtile + (int)ytile) % 2;

			// TODO: sample texture/color from xtile,ytile
			unsigned char floor_color_r = 0xff * checkerboard;
			unsigned char floor_color_g = 0x00;
			unsigned char floor_color_b = 0x00;
			unsigned char ceiling_color_r = 0x00;
			unsigned char ceiling_color_g = 0xff * checkerboard;
			unsigned char ceiling_color_b = 0x00;

			int pixels_index = 4 * (row * num_columns + column);
			pixels[pixels_index + 0] = floor_color_r;
			pixels[pixels_index + 1] = floor_color_g;
			pixels[pixels_index + 2] = floor_color_b;
			pixels[pixels_index + 3] = 0xff;

			pixels_index = 4 * ((num_rows - row - 1) * num_columns + column);
			pixels[pixels_index + 0] = ceiling_color_r;
			pixels[pixels_index + 1] = ceiling_color_g;
			pixels[pixels_index + 2] = ceiling_color_b;
			pixels[pixels_index + 3] = 0xff;
		}
	}

	// Draw walls
	for (int column = 0; column < num_columns; column++) {

		// Find distance from nearest wall to camera plane
		double ray_offset = ((2.0 * column / num_columns) - 1) * renderer->fov;
		double ray_rx = cos(r) - sin(r) * ray_offset;
		double ray_ry = sin(r) + cos(r) * ray_offset;
		struct ray_result hit = rc_renderer_internal_raycast(map, x, y, atan2(ray_ry, ray_rx));
		hit.distance *= 1 / sqrt(ray_rx * ray_rx + ray_ry * ray_ry);

		// Determine length and y position of this column
		int line_length = num_rows * renderer->wallheight / (hit.distance * renderer->fov);
		if (line_length > num_rows)
			line_length = num_rows;
		int line_height = (num_rows - line_length) / 2;

		// TODO: get color and texture info about hit.wall
		int pixels_index = 4 * (line_height * num_columns + column);
		for (int i = 0; i < line_length; i++) {
			pixels[pixels_index + 0] = 0xff * (hit.wall % 3);
			pixels[pixels_index + 1] = 0xff * (hit.wall % 4);
			pixels[pixels_index + 2] = 0xff * (hit.wall % 5);
			pixels[pixels_index + 3] = 0xff;
			pixels_index += num_columns * 4;
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

static void rc_renderer_internal_resize(struct raycaster_renderer *renderer) {
	int num_columns = renderer->width / renderer->pixelation;
	int num_rows = renderer->height / renderer->pixelation;

	// Generate an empty frame to use as the next frame with a new PBO
	unsigned char blank_frame[4 * num_columns * num_rows];
	for (int i = 0; i < 4 * num_columns * num_rows; i++)
		blank_frame[i] = 0;
	renderer->current_pbo = 0;
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, renderer->double_pbo[renderer->current_pbo]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, 4 * sizeof (unsigned char) * num_columns * num_rows, blank_frame, GL_STREAM_DRAW);

    glBindTexture(GL_TEXTURE_2D, renderer->tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, num_columns, num_rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static struct ray_result rc_renderer_internal_raycast(struct raycaster_map *map, double x, double y, double a) {
	double dx = 1 / cos(a), dy = 1 / sin(a);
	double rx = x - (int)x, ry = y - (int)y;

	int sx = -1, sy = -1;
	if (dx >= 0) { rx = 1 - rx; sx = 1; }
	if (dy >= 0) { ry = 1 - ry; sy = 1; }

	dx = fabs(dx); dy = fabs(dy);
	rx *= dx; ry *= dy;

	struct ray_result hit = { 0, 0, 0 };
	while (!(hit.wall = rc_map_get_wall(map, x, y))) {
		if (rx < ry) {
			hit.distance = rx;
			hit.direction = 1;
			rx += dx;
			x += sx;
		} else {
			hit.distance = ry;
			hit.direction = 0;
			ry += dy;
			y += sy;
		}
	}

	return hit;
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
