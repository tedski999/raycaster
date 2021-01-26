#include "renderer.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <glad/glad.h>

struct raycaster_renderer {
	// Settings
	int width, height;
	double aspect;
	double fov;
	int pixelation;
	double wallheight;
	// OpenGL buffers
	int num_columns, num_rows;
	unsigned column_vbo, column_ibo, column_vao, column_shader; // walls
	unsigned fc_vbo, fc_ibo, fc_vao, fc_texture, fc_shader;     // floorceiling
	double *column_vertices;  // num_columns * 4 * 8 doubles
	unsigned char *fc_pixels; // num_columns * num_rows * 4 bytes
};

struct ray_result {
	double distance;
	int direction;
	int wall;
};

static struct ray_result renderer_internal_raycast(const int *const map, int map_width, int map_height, double x, double y, double a);
static void renderer_internal_resize_buffers(struct raycaster_renderer *renderer);
static unsigned renderer_internal_create_shader(const char *const filepath, GLenum shader_type);
static unsigned renderer_internal_create_shader_program(unsigned shaders[], int count);
static void renderer_internal_opengl_message_callback(GLenum source, GLenum type, unsigned id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

struct raycaster_renderer *renderer_create(int width, int height, double aspect, double fov, int pixelation, double wallheight) {

	// Load OpenGL functions for the current context
	gladLoadGL();
	if (!GLAD_GL_VERSION_3_2) {
		fprintf(stderr, "Your system doesn't support OpenGL >= 3.2!\n");
		exit(1);
	}
	printf("%s - %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Configure OpenGL
	glEnable(GL_DEBUG_OUTPUT); // TODO: togglable
	glDebugMessageCallback(renderer_internal_opengl_message_callback, 0);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// Allocate renderer
	struct raycaster_renderer *renderer = malloc(sizeof *renderer);
	// TODO: assert malloc
	*renderer = (struct raycaster_renderer) {
		width, height, aspect, fov, pixelation, wallheight,
		width / pixelation, height / pixelation
	};

	// Column buffers
	glGenBuffers(1, &renderer->column_vbo);
	glGenBuffers(1, &renderer->column_ibo);
	glGenVertexArrays(1, &renderer->column_vao);
	unsigned column_shaders[2];
	column_shaders[0] = renderer_internal_create_shader("res/shaders/column.vert", GL_VERTEX_SHADER);
	column_shaders[1] = renderer_internal_create_shader("res/shaders/column.frag", GL_FRAGMENT_SHADER);
	renderer->column_shader = renderer_internal_create_shader_program(column_shaders, 2);
	glDeleteShader(column_shaders[0]);
	glDeleteShader(column_shaders[1]);

	// Floorceiling buffers
	glGenBuffers(1, &renderer->fc_vbo);
	glGenBuffers(1, &renderer->fc_ibo);
	glGenTextures(1, &renderer->fc_texture);
	glGenVertexArrays(1, &renderer->fc_vao);
	glBindVertexArray(renderer->fc_vao);
	glBindBuffer(GL_ARRAY_BUFFER, renderer->fc_vbo);
	double fc_vertices[] = {
		 1.0,  1.0, 1.0, 1.0,
		 1.0, -1.0, 1.0, 0.0,
		-1.0, -1.0, 0.0, 0.0,
		-1.0,  1.0, 0.0, 1.0
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof fc_vertices, fc_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 4 * sizeof (double), (void *) 0);
	glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, 4 * sizeof (double), (void *) (2 * sizeof (double)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	unsigned quad_indices[] = { 0, 1, 3, 1, 2, 3 };
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->fc_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof quad_indices, quad_indices, GL_STATIC_DRAW);
	glBindVertexArray(0);
	unsigned fc_shaders[2];
	fc_shaders[0] = renderer_internal_create_shader("res/shaders/floorceiling.vert", GL_VERTEX_SHADER);
	fc_shaders[1] = renderer_internal_create_shader("res/shaders/floorceiling.frag", GL_FRAGMENT_SHADER);
	renderer->fc_shader = renderer_internal_create_shader_program(fc_shaders, 2);
	glDeleteShader(fc_shaders[0]);
	glDeleteShader(fc_shaders[1]);

	return renderer;
}

void renderer_clear() {
	glClear(GL_COLOR_BUFFER_BIT);
}

void renderer_set_dimensions(struct raycaster_renderer *renderer, int width, int height) {
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
	renderer->num_columns = w / renderer->pixelation;
	renderer->num_rows = h / renderer->pixelation;
	glViewport(x, y, w, h);
	renderer_internal_resize_buffers(renderer);
}

void renderer_set_fov(struct raycaster_renderer *renderer, double fov) {
	renderer->fov = fov;
}

void renderer_set_pixelation(struct raycaster_renderer *renderer, int pixelation) {
	renderer->num_columns = renderer->width / pixelation;
	renderer->num_rows = renderer->height / pixelation;
	renderer->pixelation = pixelation;
	renderer_internal_resize_buffers(renderer);
}

void renderer_set_wallheight(struct raycaster_renderer *renderer, double wallheight) {
	renderer->wallheight = wallheight;
}

void renderer_draw_floorceiling(struct raycaster_renderer *renderer, const int *const map, int map_width, int map_height, double x, double y, double r) {

	// Compute each pixels new color in fc_pixels from this view
	double ray_left_rx = cos(r) + sin(r) * renderer->fov, ray_left_ry = sin(r) - cos(r) * renderer->fov;
	double ray_right_rx = cos(r) - sin(r) * renderer->fov, ray_right_ry = sin(r) + cos(r) * renderer->fov;
	for (int row = 0; row < renderer->num_rows / 2; row++) {
		double row_dist = renderer->wallheight * renderer->num_rows / (renderer->num_rows - 2.0 * row);
		row_dist *= 1.0 / renderer->fov;

		for (int column = 0; column < renderer->num_columns; column++) {
			double angle = (double)column / renderer->num_columns;
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

			int buffer_index = 4 * (row * renderer->num_columns + column);
			renderer->fc_pixels[buffer_index + 0] = floor_color_r;
			renderer->fc_pixels[buffer_index + 1] = floor_color_g;
			renderer->fc_pixels[buffer_index + 2] = floor_color_b;
			renderer->fc_pixels[buffer_index + 3] = 0xff;

			buffer_index = 4 * ((renderer->num_rows - row - 1) * renderer->num_columns + column);
			renderer->fc_pixels[buffer_index + 0] = ceiling_color_r;
			renderer->fc_pixels[buffer_index + 1] = ceiling_color_g;
			renderer->fc_pixels[buffer_index + 2] = ceiling_color_b;
			renderer->fc_pixels[buffer_index + 3] = 0xff;
		}
	}

	// Draw floor and ceiling by overwriting fc_texture with fc_pixels and
	// drawing a textured quad over the viewpoint
	glUseProgram(renderer->fc_shader);
	glBindTexture(GL_TEXTURE_2D, renderer->fc_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, renderer->num_columns, renderer->num_rows, GL_RGBA, GL_UNSIGNED_BYTE, renderer->fc_pixels);
	glBindVertexArray(renderer->fc_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void renderer_draw_walls(struct raycaster_renderer *renderer, const int *const map, int map_width, int map_height, double x, double y, double r) {

	// Compute the wall heights for the new column_vertices
	double column_width = 2.0 / renderer->num_columns;
	for (int column = 0; column < renderer->num_columns; column++) {
		double screen_x = (2.0 * column / renderer->num_columns) - 1;

		// Find distance from nearest wall to camera plane
		double ray_offset = screen_x * renderer->fov;
		double ray_rx = cos(r) - sin(r) * ray_offset;
		double ray_ry = sin(r) + cos(r) * ray_offset;
		struct ray_result hit = renderer_internal_raycast(map, map_width, map_height, x, y, atan2(ray_ry, ray_rx));
		hit.distance *= 1 / sqrt(ray_rx * ray_rx + ray_ry * ray_ry);

		// TODO: get color and texture info about hit.wall
		double wall_color_r = 1.0 * (hit.wall % 3);
		double wall_color_g = 1.0 * (hit.wall % 4);
		double wall_color_b = 1.0 * (hit.wall % 5);

		// Determine new vertices for this column
		double line_height = renderer->wallheight / hit.distance;
		line_height *= 1.0 / renderer->fov;
		line_height = (int)(line_height * renderer->num_rows) / (double)renderer->num_rows;
		double new_vertices[4 * 8] = {
			screen_x,                line_height,  wall_color_r, wall_color_g, wall_color_b, 1.00, 0.00, 0.00,
			screen_x,                -line_height, wall_color_r, wall_color_g, wall_color_b, 1.00, 0.00, 0.00,
			screen_x + column_width, -line_height, wall_color_r, wall_color_g, wall_color_b, 1.00, 0.00, 0.00,
			screen_x + column_width, line_height,  wall_color_r, wall_color_g, wall_color_b, 1.00, 0.00, 0.00
		};

		// Write new_vertices to column_vertices
		for (int i = 0; i < 4 * 8; i++)
			renderer->column_vertices[column * 4 * 8 + i] = new_vertices[i];
	}

	// Draw the new column_vertices
	glUseProgram(renderer->column_shader);
	glNamedBufferSubData(renderer->column_vbo, 0, 4 * 8 * sizeof (double) * renderer->num_columns, renderer->column_vertices);
	glBindVertexArray(renderer->column_vao);
	glDrawElements(GL_TRIANGLES, 6 * renderer->num_columns, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void renderer_destroy(struct raycaster_renderer *renderer) {
	glDeleteBuffers(1, &renderer->column_vbo);
	glDeleteBuffers(1, &renderer->column_ibo);
	glDeleteVertexArrays(1, &renderer->column_vao);
	glDeleteProgram(renderer->column_shader);
	glDeleteBuffers(1, &renderer->fc_vbo);
	glDeleteBuffers(1, &renderer->fc_ibo);
	glDeleteTextures(1, &renderer->fc_texture);
	glDeleteVertexArrays(1, &renderer->fc_vao);
	glDeleteProgram(renderer->fc_shader);
	free(renderer->column_vertices);
	free(renderer->fc_pixels);
}

static void renderer_internal_resize_buffers(struct raycaster_renderer *renderer) {

	// Allocate and initialize the column buffers for this new resolution
	renderer->column_vertices = realloc(renderer->column_vertices, 4 * 8 * sizeof (double) * renderer->num_columns);
	glBindVertexArray(renderer->column_vao);
	// Setup new column_vbo
	glBindBuffer(GL_ARRAY_BUFFER, renderer->column_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * 8 * sizeof (double) * renderer->num_columns, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 8 * sizeof (double), (void *) 0);
	glVertexAttribPointer(1, 4, GL_DOUBLE, GL_FALSE, 8 * sizeof (double), (void *) (2 * sizeof (double))); // TODO: colors don't need double types - unsigned char will do
	glVertexAttribPointer(2, 2, GL_DOUBLE, GL_FALSE, 8 * sizeof (double), (void *) (6 * sizeof (double)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Setup new column_ibo
	unsigned indices[6 * renderer->num_columns];
	unsigned quad_indices[6] = { 0, 1, 3, 1, 2, 3 };
	for (int i = 0; i < renderer->num_columns; i++)
		for (int j = 0; j < 6; j++)
			indices[i * 6 + j] = i * 4 + quad_indices[j];
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->column_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
	// Done!
	glBindVertexArray(0);

	// Allocate and initialize the floorceiling texture buffers for this new resolution
	// FIXME: Max texture width or height is meant to be 1024! We will probably need to split the textures up if resolution is too large
	renderer->fc_pixels = realloc(renderer->fc_pixels, 4 * sizeof (unsigned char) * renderer->num_columns * renderer->num_rows);
	glBindTexture(GL_TEXTURE_2D, renderer->fc_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderer->num_columns, renderer->num_rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}

static struct ray_result renderer_internal_raycast(const int *const map, int map_height, int map_width, double x, double y, double a) {
	double dx = 1 / cos(a), dy = 1 / sin(a);
	double rx = x - (int)x, ry = y - (int)y;

	int sx = -1, sy = -1;
	if (dx >= 0) { rx = 1 - rx; sx = 1; }
	if (dy >= 0) { ry = 1 - ry; sy = 1; }

	dx = fabs(dx); dy = fabs(dy);
	rx *= dx; ry *= dy;

	struct ray_result hit = { 0, 0, 0 };
	while (x >= 0 & x < map_width & y >= 0 & y < map_height & !(hit.wall = map[(int)y * map_width + (int)x])) {
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

static unsigned renderer_internal_create_shader(const char *const filepath, GLenum shader_type) {
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

static unsigned renderer_internal_create_shader_program(unsigned shaders[], int count) {
	unsigned program = glCreateProgram();
	for (int i = 0; i < count; i++)
		glAttachShader(program, shaders[i]);
	glLinkProgram(program);
	return program;
}

static void renderer_internal_opengl_message_callback(GLenum source, GLenum type, unsigned id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	fprintf(stderr, "%s %s\n", (type == GL_DEBUG_TYPE_ERROR) ? "(GL ERR):" : "(GL)", message);
}
