#include "renderer.h"
#include "error.h"
#include "window.h"
#include "map.h"
#include "entity.h"
#include "texture.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct rc_renderer {
	const struct rc_window *window;
	double aspect, fov;
	struct rc_texture **wall_textures;
	int num_columns, num_rows;
	double *zbuffer;
	unsigned vao, vbo, ibo;
	unsigned tex, double_pbo[2], shader;
	int current_pbo;
};

static void rc_renderer_internal_raycast(const struct rc_map *map, double x, double y, double a, int *hit_x, int *hit_y, int *hit_side, double *hit_dst, double *hit_lat);
static void rc_renderer_internal_initialize_opengl(struct rc_renderer *renderer);
static void rc_renderer_internal_resize_opengl_buffers(struct rc_renderer *renderer);
static unsigned rc_renderer_internal_create_shader(const char *filepath, GLenum shader_type);
static unsigned rc_renderer_internal_create_shader_program(const unsigned shaders[], int count);
#ifdef RC_DEBUG
static void rc_renderer_internal_opengl_message_callback(GLenum source, GLenum type, unsigned id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
#endif

struct rc_renderer *rc_renderer_create(const struct rc_window *window, double aspect, int resolution, double fov, struct rc_texture **wall_textures) {
	struct rc_renderer *renderer = malloc(sizeof *renderer);
	RC_ASSERT(renderer);
	*renderer = (struct rc_renderer) { window, aspect };
	rc_renderer_internal_initialize_opengl(renderer);
	rc_renderer_set_fov(renderer, fov);
	rc_renderer_set_wall_textures(renderer, wall_textures);
	rc_renderer_set_resolution(renderer, resolution);
	rc_window_set_renderer(window, renderer);
	return renderer;
}

void rc_renderer_set_dimensions(const struct rc_renderer *renderer, int width, int height) {
	double xratio = renderer->aspect * height / width;
	double yratio = 1 / xratio;
	if (xratio > 1) xratio = 1;
	if (yratio > 1) yratio = 1;

	const double x = width / 2.0 * (1 - xratio);
	const double y = height / 2.0 * (1 - yratio);
	const double w = width * xratio;
	const double h = height * yratio;
	glViewport(x, y, w, h);
}

void rc_renderer_set_fov(struct rc_renderer *renderer, double fov) {
	renderer->fov = fov;
}

void rc_renderer_set_resolution(struct rc_renderer *renderer, int resolution) {
	RC_ASSERT(resolution > 1);
	renderer->num_columns = renderer->aspect * resolution;
	renderer->num_rows = resolution;
	rc_renderer_internal_resize_opengl_buffers(renderer);
}

void rc_renderer_set_wall_textures(struct rc_renderer *renderer, struct rc_texture **wall_textures) {
	renderer->wall_textures = wall_textures;
}

void rc_renderer_draw(struct rc_renderer *renderer, const struct rc_map *map, struct rc_entity **entities, int entities_count, const struct rc_entity *camera) {

	// Render with the current PBO
	glClear(GL_COLOR_BUFFER_BIT);
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

	// Prepare for drawing
	int map_width, map_height;
	double cam_x, cam_y, cam_z, cam_r;
	rc_map_get_size(map, &map_width, &map_height);
	rc_entity_get_transform(camera, &cam_x, &cam_y, &cam_z, &cam_r);

	// Draw floor and ceiling
	const double ray_rx = cos(cam_r) + sin(cam_r) * renderer->fov;
	const double ray_ry = sin(cam_r) - cos(cam_r) * renderer->fov;
	const double xtiles_per_column = 2 * renderer->fov * sin(-cam_r) / renderer->num_columns;
	const double ytiles_per_column = 2 * renderer->fov * cos( cam_r) / renderer->num_columns;
	for (int row = 0; row < renderer->num_rows; row++) {
		const bool is_floor = row < renderer->num_rows / 2;

		// Draw the column of pixels for this row by sampling the floor/ceiling
		// textures for all the tiles crossed by this stepping ray
		const double row_angle = renderer->num_rows - 2 * row;
		const double row_dst = 2 * renderer->num_rows / renderer->fov * ((is_floor) ? cam_z / row_angle : (1 - cam_z) / (1 - row_angle));
		const double ray_step_x = row_dst * xtiles_per_column, ray_step_y = row_dst * ytiles_per_column;
		double ray_x = cam_x + row_dst * ray_rx, ray_y = cam_y + row_dst * ray_ry;
		for (int column = 0; column < renderer->num_columns; column++) {

			// Find the current tile and the position within this tile of the ray
			const int tile_x = ray_x, tile_y = ray_y;
			const double tile_offset_x = ray_x - tile_x, tile_offset_y = ray_y - tile_y;
			ray_x += ray_step_x; ray_y += ray_step_y;

			// Don't draw tiles outside the map
			if (tile_x < 0 || tile_x >= map_width || tile_y < 0 || tile_y >= map_height)
				continue;

			// Get the pixel color of the position of the ray
			int tex_width, tex_height;
			unsigned char light_r, light_g, light_b;
			unsigned char color_r, color_g, color_b, color_a;
			rc_map_get_lighting(map, tile_x + tile_offset_x, tile_y + tile_offset_y, &light_r, &light_g, &light_b);
			const int tex_index = (is_floor) ? rc_map_get_floor(map, tile_x, tile_y) : rc_map_get_ceiling(map, tile_x, tile_y);
			const struct rc_texture *tex = renderer->wall_textures[tex_index];
			rc_texture_get_dimensions(tex, &tex_width, &tex_height);
			const int tex_x = tex_width * tile_offset_x, tex_y = tex_height * tile_offset_y;
			rc_texture_get_pixel(tex, tex_x, tex_y, &color_r, &color_g, &color_b, &color_a);

			// Draw pixel to the screen
			const int pixels_index = 4 * (row * renderer->num_columns + column);
			pixels[pixels_index + 0] = color_r * light_r / 0xffp0;
			pixels[pixels_index + 1] = color_g * light_g / 0xffp0;
			pixels[pixels_index + 2] = color_b * light_b / 0xffp0;
			pixels[pixels_index + 3] = color_a;
		}
	}

	// Draw walls
	for (int column = 0; column < renderer->num_columns; column++) {

		// Find distance from nearest wall to camera plane
		int hit_x, hit_y, hit_side;
		double hit_dst, hit_lat;
		const double ray_offset = (2.0 * column / renderer->num_columns - 1) * renderer->fov;
		const double ray_rx = cos(cam_r) - sin(cam_r) * ray_offset;
		const double ray_ry = sin(cam_r) + cos(cam_r) * ray_offset;
		rc_renderer_internal_raycast(map, cam_x, cam_y, atan2(ray_ry, ray_rx), &hit_x, &hit_y, &hit_side, &hit_dst, &hit_lat);
		hit_dst *= 1 / sqrt(ray_rx * ray_rx + ray_ry * ray_ry);
		renderer->zbuffer[column] = hit_dst;

		// Don't draw empty walls
		const int hit_wall = rc_map_get_wall(map, hit_x, hit_y);
		if (hit_wall == -1)
			continue;

		// Determine the length and position of the column to be drawn as a vertical line
		const double column_length = 1 / hit_dst / renderer->fov;
		const double column_offset = (cam_z - 0.5) / hit_dst / renderer->fov;
		const double lower_bound = (1 - column_length) / 2 - column_offset;
		const double upper_bound = (1 + column_length) / 2 - column_offset;
		const int first_row = round(fmax(lower_bound, 0) * renderer->num_rows);
		const int texture_row = round(fmax(-lower_bound, 0) * renderer->num_rows);
		const int last_row = round(fmin(upper_bound, 1) * renderer->num_rows);

		// Find the starting point to sample from and the distance between each sample for the texture of the column to be drawn
		int tex_width, tex_height;
		const struct rc_texture *tex = renderer->wall_textures[hit_wall];
		rc_texture_get_dimensions(tex, &tex_width, &tex_height);
		const double texels_per_row = tex_height / (column_length * renderer->num_rows + 1);
		double tex_x = hit_lat * tex_width, tex_y = tex_height - texture_row * texels_per_row - 1;

		// Texture flipping
		if ((hit_side && ray_rx < 0) || (!hit_side && ray_ry > 0))
			tex_x = tex_width - tex_x;

		// Sample lighting from tile adjacent to surface rather than the tile of the wall hit
		if (hit_side) (ray_rx < 0) ? hit_x++ : hit_x--;
		else          (ray_ry < 0) ? hit_y++ : hit_y--;

		// Draw a vertical line for the wall column
		for (int row = first_row; row < last_row; row++) {

			// Sample texture
			unsigned char light_r, light_g, light_b;
			unsigned char color_r, color_g, color_b, color_a;
			rc_map_get_lighting(map, hit_x, hit_y, &light_r, &light_g, &light_b);
			rc_texture_get_pixel(tex, tex_x, tex_y, &color_r, &color_g, &color_b, &color_a);
			tex_y -= texels_per_row;

			// Fill in the pixel in the PBO
			const int pixels_index = 4 * (row * renderer->num_columns + column);
			pixels[pixels_index + 0] = color_r * light_r / 0xffp0;
			pixels[pixels_index + 1] = color_g * light_g / 0xffp0;
			pixels[pixels_index + 2] = color_b * light_b / 0xffp0;
			pixels[pixels_index + 3] = color_a;
		}
	}

	// Draw entities
	// TODO: sort entities here
	for (int i = 0; i < entities_count; i++) {
		const struct rc_texture *tex = rc_entity_get_texture(entities[i]);

		// Skip untextured entities
		if (!tex)
			continue;

		// Get entity transformation and lighting
		double entity_x, entity_y, entity_z, entity_r, entity_s = 1.0; // TODO: entity scaling
		unsigned char light_r, light_g, light_b;
		rc_entity_get_transform(entities[i], &entity_x, &entity_y, &entity_z, &entity_r);
		rc_map_get_lighting(map, entity_x, entity_y, &light_r, &light_g, &light_b);

		// Calculate entitys transformation relative to camera
		const double entity_offset_x = entity_x - cam_x, entity_offset_y = entity_y - cam_y, entity_offset_z = entity_z - cam_z;
		const double entity_transform_x = entity_offset_y * cos(cam_r) - entity_offset_x * sin(cam_r);
		const double entity_transform_y = entity_offset_x * cos(cam_r) + entity_offset_y * sin(cam_r);

		// Skip entities behind camera view
		if (entity_transform_y < 0)
			continue;

		// Calculate the transformation of the entitys texture on-screen
		const double texture_x_scaling = renderer->num_columns * (1 + entity_transform_x / entity_transform_y / renderer->fov);
		const double texture_y_scaling = renderer->num_rows * entity_s / entity_transform_y / renderer->fov;
		const double texture_z_scaling = renderer->num_rows * entity_offset_z / entity_transform_y / renderer->fov;

		// Calculate common scaling calculations - these are generally used to figure out screen texture boundaries
		const double x_lower_bound = (texture_x_scaling - texture_y_scaling) / 2;
		const double x_upper_bound = (texture_x_scaling + texture_y_scaling) / 2;
		const double y_lower_bound = (renderer->num_rows - texture_y_scaling) / 2;
		const double y_upper_bound = (renderer->num_rows + texture_y_scaling) / 2;

		// Calculate screen pixel coordinates of texture
		// first to last is the range of the texture on-screen
		// tex_base accounts for the texture beginning off-screen
		const int first_column    = round(fmax(x_lower_bound, 0));
		const int tex_base_column = round(fmax(-x_lower_bound, 0));
		const int last_column     = round(fmin(x_upper_bound, renderer->num_columns));
		const int first_row       = round(fmax(y_lower_bound + texture_z_scaling, 0));
		const int tex_base_row    = round(fmax(-y_lower_bound - texture_z_scaling, 0));
		const int last_row        = round(fmin(y_upper_bound + texture_z_scaling, renderer->num_rows));

		// Calculate distances between each sample along the column or row
		int tex_width, tex_height;
		rc_texture_get_dimensions(tex, &tex_width, &tex_height);
		const double texels_per_column = tex_width / (x_upper_bound - x_lower_bound);
		const double texels_per_row = tex_height / (y_upper_bound - y_lower_bound);

		// Iterate over every column on the screen that contains the texture being drawn
		for (int column = first_column; column < last_column; column++) {

			// Skip the column if the texture is hidden behind a wall
			if (entity_transform_y > renderer->zbuffer[column])
				continue;

			// Iterate over every row on the screen that contains the texture being drawn
			for (int row = first_row; row < last_row; row++) {

				// Calculate texture sample point
				const int tex_x = (column - first_column + tex_base_column) * texels_per_column;
				const int tex_y = tex_height - (row - first_row + tex_base_row) * texels_per_row - 1;

				// Sample texture
				// TODO: used RBGA instead of RBG textures, currently black is a transparent pixel
				unsigned char color_r, color_g, color_b, color_a;
				rc_texture_get_pixel(tex, tex_x, tex_y, &color_r, &color_g, &color_b, &color_a);
				if (!color_r && !color_g && !color_b)
					continue;

				// Fill in the pixel in the PBO
				const int pixels_index = 4 * (row * renderer->num_columns + column);
				pixels[pixels_index + 0] = color_r * light_r / 0xffp0;
				pixels[pixels_index + 1] = color_g * light_g / 0xffp0;
				pixels[pixels_index + 2] = color_b * light_b / 0xffp0;
				pixels[pixels_index + 3] = color_a;
			}
		}
	}

	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void rc_renderer_destroy(struct rc_renderer *renderer) {
	glDeleteVertexArrays(1, &renderer->vao);
	glDeleteBuffers(1, &renderer->vbo);
	glDeleteBuffers(1, &renderer->ibo);
	glDeleteBuffers(2, renderer->double_pbo);
	glDeleteTextures(1, &renderer->tex);
	glDeleteProgram(renderer->shader);
	free(renderer->zbuffer);
}

static void rc_renderer_internal_raycast(const struct rc_map *map, double x, double y, double a, int *hit_x, int *hit_y, int *hit_side, double *hit_dst, double *hit_lat) {

	// Starting point
	*hit_x = x, *hit_y = y;
	double distance_x = x - *hit_x, distance_y = y - *hit_y;

	// Direction vector
	const double angle_x = cos(a), angle_y = sin(a);
	const double delta_x = fabs(1 / angle_x), delta_y = fabs(1 / angle_y);

	// Flip step vector for negative directions
	int step_x = -1, step_y = -1;
	if (angle_x >= 0) { distance_x = 1 - distance_x; step_x = 1; }
	if (angle_y >= 0) { distance_y = 1 - distance_y; step_y = 1; }
	distance_x *= delta_x; distance_y *= delta_y;

	// Euclidean distance to the nearest wall
	*hit_side = 0;
	while (rc_map_get_wall(map, *hit_x, *hit_y) == -1) {
		if (distance_x < distance_y) {
			*hit_side = 1;
			*hit_x += step_x;
			distance_x += delta_x;
		} else {
			*hit_side = 0;
			*hit_y += step_y;
			distance_y += delta_y;
		}
	}

	// Calculate distance and point on the wall surface
	*hit_dst = (*hit_side) ? distance_x - delta_x : distance_y - delta_y;
	*hit_lat = (*hit_side) ? y + angle_y * *hit_dst : x + angle_x * *hit_dst;
	*hit_lat -= (int)*hit_lat;
}

static void rc_renderer_internal_initialize_opengl(struct rc_renderer *renderer) {

	// Load OpenGL functions for the current context
	gladLoadGL(glfwGetProcAddress);
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
	const unsigned quad_indices[] = { 0, 1, 3, 1, 2, 3 };
	const double quad_vertices[] = {
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

static void rc_renderer_internal_resize_opengl_buffers(struct rc_renderer *renderer) {

	// Allocate the first PBO - The second one is allocated during the next render
	// Zero out the new PBO so we don't display garbage for the next frame
	renderer->current_pbo = 0;
	unsigned char *blank_frame = calloc(4 * renderer->num_columns * renderer->num_rows, sizeof *blank_frame);
	RC_ASSERT(blank_frame);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, renderer->double_pbo[renderer->current_pbo]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, 4 * sizeof (unsigned char) * renderer->num_columns * renderer->num_rows, blank_frame, GL_STREAM_DRAW);
	free(blank_frame);

	// Allocate the texture object buffer - Also transfer the new PBO into it
	glBindTexture(GL_TEXTURE_2D, renderer->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderer->num_columns, renderer->num_rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // TODO: watch out for max texture size

	// Done - Unbind buffers
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	// Resize the zbuffer
	double *new_zbuffer  = realloc(renderer->zbuffer, sizeof *new_zbuffer * renderer->num_columns);
	RC_ASSERT(new_zbuffer);
	renderer->zbuffer = new_zbuffer;
}

static unsigned rc_renderer_internal_create_shader(const char *filepath, GLenum shader_type) {
	FILE *file = fopen(filepath, "r");
	if (!file) {
		fprintf(stderr, "Could not read from '%s'!\n", filepath);
		exit(1);
	}

	fseek(file , 0, SEEK_END);
	const int file_size = ftell(file);
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

	const char *source = file_buffer;
	const unsigned shader = glCreateShader(shader_type);
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

static unsigned rc_renderer_internal_create_shader_program(const unsigned shaders[], int count) {
	const unsigned program = glCreateProgram();
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
