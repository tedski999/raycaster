#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define DEG2RAD(degree) (degree * (3.14152 / 180))
#define RAD2DEG(degree) (degree * (180 / 3.14152))

#define window_title "raycaster"
#define window_aspect (16.0 / 9.0)

#define map_width 24
#define map_height 24

#define player_accel 0.01
#define player_drag 1.2
#define mouse_sensitivity 0.01

struct ray_result {
	double distance;
	int direction;
	int wall;
};

static unsigned create_shader(const char *const filepath, GLenum shader_type);
static unsigned create_shader_program(unsigned shaders[], int count);
static void render_viewpoint(const int map[map_height][map_width], double x, double y, double r);
static struct ray_result raycast(const int map[map_height][map_width], double x, double y, double a);
static void update_resolution();
static void glfw_framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void glfw_error_callback(int code, const char *desc);
static void opengl_message_callback(GLenum source, GLenum type, unsigned id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

static int window_width = 640;
static int window_height = 480;
static int pixelation = 5;
static double fov = DEG2RAD(60);
static double wall_height = 1.0;

static int num_columns = 0, num_rows = 0;
static unsigned column_vbo, column_ibo, column_vao, column_shader; // walls
static unsigned fc_vbo, fc_ibo, fc_vao, fc_texture, fc_shader; // floorceiling

// Can be Too big for stack allocation
static double *column_vertices; // num_columns * 4 * 8 doubles
static unsigned char *fc_pixels; // num_columns * num_rows * 4 bytes

int main(int argc, char **argv) {

	const int map[map_height][map_width] = {
		{4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,7,7,7,7,7,7,7,7},
		{4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
		{4,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
		{4,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
		{4,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
		{4,0,4,0,0,0,0,5,5,5,5,5,5,5,5,5,7,7,0,7,7,7,7,7},
		{4,0,5,0,0,0,0,5,0,5,0,5,0,5,0,5,7,0,0,0,7,7,7,1},
		{4,0,6,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
		{4,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,1},
		{4,0,8,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
		{4,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,7,7,7,1},
		{4,0,0,0,0,0,0,5,5,5,5,0,5,5,5,5,7,7,7,7,7,7,7,1},
		{6,6,6,6,6,6,6,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
		{8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
		{6,6,6,6,6,6,0,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
		{4,4,4,4,4,4,0,4,4,4,6,0,6,2,2,2,2,2,2,2,3,3,3,3},
		{4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
		{4,0,0,0,0,0,0,0,0,0,0,0,6,2,0,0,5,0,0,2,0,0,0,2},
		{4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
		{4,0,6,0,6,0,0,0,0,4,6,0,0,0,0,0,5,0,0,0,0,0,0,2},
		{4,0,0,5,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
		{4,0,6,0,6,0,0,0,0,4,6,0,6,2,0,0,5,0,0,2,0,0,0,2},
		{4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
		{4,4,4,4,4,4,4,4,4,4,1,1,1,2,2,2,2,2,2,3,3,3,3,3}
	};

	double player_x = 11.5, player_y = 7.5;
	double player_vx = 0, player_vy = 0;
	double player_r = DEG2RAD(90);
	double prev_mouse_x, prev_mouse_y;

	// Open window
	glfwSetErrorCallback(glfw_error_callback);
	glfwInit();
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);

	// Configure window
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
	glfwSetKeyCallback(window, glfw_key_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSwapInterval(1);

	// Load OpenGL functions
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (!GLAD_GL_VERSION_3_2) {
		fprintf(stderr, "Your system doesn't support OpenGL >= 3.2!\n");
	}
	printf("%s - %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Configure OpenGL
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(opengl_message_callback, 0);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// Column buffers
	glGenBuffers(1, &column_vbo);
	glGenBuffers(1, &column_ibo);
	glGenVertexArrays(1, &column_vao);
	unsigned column_shaders[2];
	column_shaders[0] = create_shader("res/shaders/column.vert", GL_VERTEX_SHADER);
	column_shaders[1] = create_shader("res/shaders/column.frag", GL_FRAGMENT_SHADER);
	column_shader = create_shader_program(column_shaders, 2);
	glDeleteShader(column_shaders[0]);
	glDeleteShader(column_shaders[1]);

	// Floorceiling buffers
	glGenBuffers(1, &fc_vbo);
	glGenBuffers(1, &fc_ibo);
	glGenTextures(1, &fc_texture);
	glGenVertexArrays(1, &fc_vao);
	glBindVertexArray(fc_vao);
	glBindBuffer(GL_ARRAY_BUFFER, fc_vbo);
	double vertices[] = {
		 1.0,  1.0, 1.0, 1.0,
		 1.0, -1.0, 1.0, 0.0,
		-1.0, -1.0, 0.0, 0.0,
		-1.0,  1.0, 0.0, 1.0
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 4 * sizeof (double), (void *) 0);
	glVertexAttribPointer(1, 2, GL_DOUBLE, GL_FALSE, 4 * sizeof (double), (void *) (2 * sizeof (double)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	unsigned quad_indices[] = { 0, 1, 3, 1, 2, 3 };
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fc_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof quad_indices, quad_indices, GL_STATIC_DRAW);
	glBindVertexArray(0);
	unsigned fc_shaders[2];
	fc_shaders[0] = create_shader("res/shaders/floorceiling.vert", GL_VERTEX_SHADER);
	fc_shaders[1] = create_shader("res/shaders/floorceiling.frag", GL_FRAGMENT_SHADER);
	fc_shader = create_shader_program(fc_shaders, 2);
	glDeleteShader(fc_shaders[0]);
	glDeleteShader(fc_shaders[1]);

	// Update transform matrix to handle the window resolution with its aspect ratio
	glfw_framebuffer_size_callback(window, window_width, window_height);
	glfwPollEvents();
	glfwGetCursorPos(window, &prev_mouse_x, &prev_mouse_y);

	// Main game loop
	while (!glfwWindowShouldClose(window)) {

		glfwPollEvents();

		// Poll mouse input
		double mouse_x, mouse_y, mouse_vx, mouse_vy;
		glfwGetCursorPos(window, &mouse_x, &mouse_y);
		mouse_vx = mouse_x - prev_mouse_x;
		mouse_vy = mouse_y - prev_mouse_y;
		prev_mouse_x = mouse_x;
		prev_mouse_y = mouse_y;

		// Player rotation
		player_r += mouse_vx * mouse_sensitivity;
		if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) player_r += mouse_sensitivity * 4;
		if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)  player_r -= mouse_sensitivity * 4;

		// Player movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { player_vx += cos(player_r) * player_accel; player_vy += sin(player_r) * player_accel; }
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { player_vx -= cos(player_r) * player_accel; player_vy -= sin(player_r) * player_accel; }
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { player_vx -= sin(player_r) * player_accel; player_vy += cos(player_r) * player_accel; }
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { player_vx += sin(player_r) * player_accel; player_vy -= cos(player_r) * player_accel; }
		if (map[(int)(player_y + player_vy)][(int)(player_x + player_vx)] == 0) {
			player_vx *= 1.0 / player_drag;
			player_vy *= 1.0 / player_drag;
			player_x += player_vx;
			player_y += player_vy;
		} else {
			player_vx = 0;
			player_vy = 0;
		}

		// Render
		glClear(GL_COLOR_BUFFER_BIT);
		render_viewpoint(map, player_x, player_y, player_r);
		glfwSwapBuffers(window);

		// Quitting
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	// Cleanup after yourself
	glDeleteBuffers(1, &column_vbo);
	glDeleteBuffers(1, &column_ibo);
	glDeleteVertexArrays(1, &column_vao);
	glDeleteProgram(column_shader);
	glDeleteBuffers(1, &fc_vbo);
	glDeleteBuffers(1, &fc_ibo);
	glDeleteTextures(1, &fc_texture);
	glDeleteVertexArrays(1, &fc_vao);
	glDeleteProgram(fc_shader);
	glfwTerminate();
	free(column_vertices);
	free(fc_pixels);
}

static unsigned create_shader(const char *const filepath, GLenum shader_type) {
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

static unsigned create_shader_program(unsigned shaders[], int count) {
	unsigned program = glCreateProgram();
	for (int i = 0; i < count; i++)
		glAttachShader(program, shaders[i]);
	glLinkProgram(program);
	return program;
}

static void render_viewpoint(const int map[map_height][map_width], double x, double y, double r) {

	// Compute each pixels new color in fc_pixels from this view
	double ray_left_rx = cos(r) + sin(r) * fov, ray_left_ry = sin(r) - cos(r) * fov;
	double ray_right_rx = cos(r) - sin(r) * fov, ray_right_ry = sin(r) + cos(r) * fov;
	for (int row = 0; row < num_rows / 2; row++) {
		double row_dist = wall_height * num_rows / (num_rows - 2.0 * row);
		row_dist *= 1.0 / fov;

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

			int buffer_index = 4 * (row * num_columns + column);
			fc_pixels[buffer_index + 0] = floor_color_r;
			fc_pixels[buffer_index + 1] = floor_color_g;
			fc_pixels[buffer_index + 2] = floor_color_b;
			fc_pixels[buffer_index + 3] = 0xff;

			buffer_index = 4 * ((num_rows - row - 1) * num_columns + column);
			fc_pixels[buffer_index + 0] = ceiling_color_r;
			fc_pixels[buffer_index + 1] = ceiling_color_g;
			fc_pixels[buffer_index + 2] = ceiling_color_b;
			fc_pixels[buffer_index + 3] = 0xff;
		}
	}

	// Draw floor and ceiling by overwriting fc_texture with fc_pixels and
	// drawing a textured quad over the viewpoint
	glUseProgram(fc_shader);
	glBindTexture(GL_TEXTURE_2D, fc_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, num_columns, num_rows, GL_RGBA, GL_UNSIGNED_BYTE, fc_pixels);
	glBindVertexArray(fc_vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// Compute the wall heights for the new column_vertices
	double column_width = 2.0 / num_columns;
	for (int column = 0; column < num_columns; column++) {
		double screen_x = (2.0 * column / num_columns) - 1;

		// Find distance from nearest wall to camera plane
		double ray_offset = screen_x * fov;
		double ray_rx = cos(r) - sin(r) * ray_offset;
		double ray_ry = sin(r) + cos(r) * ray_offset;
		struct ray_result hit = raycast(map, x, y, atan2(ray_ry, ray_rx));
		hit.distance *= 1 / sqrt(ray_rx * ray_rx + ray_ry * ray_ry);

		// TODO: get color and texture info about hit.wall
		double wall_color_r = 1.0 * (hit.wall % 3);
		double wall_color_g = 1.0 * (hit.wall % 4);
		double wall_color_b = 1.0 * (hit.wall % 5);

		// Determine new vertices for this column
		double line_height = wall_height / hit.distance;
		line_height *= 1.0 / fov;
		line_height = (int)(line_height * num_rows) / (double)num_rows;
		double new_vertices[4 * 8] = {
			screen_x,                line_height,  wall_color_r, wall_color_g, wall_color_b, 1.00, 0.00, 0.00,
			screen_x,                -line_height, wall_color_r, wall_color_g, wall_color_b, 1.00, 0.00, 0.00,
			screen_x + column_width, -line_height, wall_color_r, wall_color_g, wall_color_b, 1.00, 0.00, 0.00,
			screen_x + column_width, line_height,  wall_color_r, wall_color_g, wall_color_b, 1.00, 0.00, 0.00
		};

		// Write new_vertices to column_vertices
		for (int i = 0; i < 4 * 8; i++)
			column_vertices[column * 4 * 8 + i] = new_vertices[i];
	}

	// Draw the new column_vertices
	glUseProgram(column_shader);
	glNamedBufferSubData(column_vbo, 0, 4 * 8 * sizeof (double) * num_columns, column_vertices);
	glBindVertexArray(column_vao);
	glDrawElements(GL_TRIANGLES, 6 * num_columns, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

static struct ray_result raycast(const int map[map_height][map_width], double x, double y, double a) {
	double dx = 1 / cos(a), dy = 1 / sin(a);
	double rx = x - (int)x, ry = y - (int)y;

	int sx = -1, sy = -1;
	if (dx >= 0) { rx = 1 - rx; sx = 1; }
	if (dy >= 0) { ry = 1 - ry; sy = 1; }

	dx = fabs(dx); dy = fabs(dy);
	rx *= dx; ry *= dy;

	struct ray_result hit = { 0, 0, 0 };
	while (x >= 0 & x < map_width & y >= 0 & y < map_height & !(hit.wall = map[(int)y][(int)x])) {
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

static void update_resolution() {
	num_columns = window_width / pixelation;
	num_rows = window_height / pixelation;

	// Allocate and initialize the column buffers for this new resolution
	column_vertices = realloc(column_vertices, 4 * 8 * sizeof (double) * num_columns);
	glBindVertexArray(column_vao);
	// Setup new column_vbo
	glBindBuffer(GL_ARRAY_BUFFER, column_vbo);
	glBufferData(GL_ARRAY_BUFFER, 4 * 8 * sizeof (double) * num_columns, NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_DOUBLE, GL_FALSE, 8 * sizeof (double), (void *) 0);
	glVertexAttribPointer(1, 4, GL_DOUBLE, GL_FALSE, 8 * sizeof (double), (void *) (2 * sizeof (double))); // TODO: colors don't need double types - unsigned char will do
	glVertexAttribPointer(2, 2, GL_DOUBLE, GL_FALSE, 8 * sizeof (double), (void *) (6 * sizeof (double)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// Setup new column_ibo
	unsigned indices[6 * num_columns];
	unsigned quad_indices[6] = { 0, 1, 3, 1, 2, 3 };
	for (int i = 0; i < num_columns; i++)
		for (int j = 0; j < 6; j++)
			indices[i * 6 + j] = i * 4 + quad_indices[j];
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, column_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
	// Done!
	glBindVertexArray(0);

	// Allocate and initialize the floorceiling texture buffers for this new resolution
	// FIXME: Max texture width or height is meant to be 1024! We will probably need to split the textures up if resolution is too large
	fc_pixels = realloc(fc_pixels, 4 * sizeof (unsigned char) * num_columns * num_rows);
	glBindTexture(GL_TEXTURE_2D, fc_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, num_columns, num_rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}

static void glfw_framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	double xratio = window_aspect * height / width;
	double yratio = 1 / xratio;
	if (xratio > 1) xratio = 1;
	if (yratio > 1) yratio = 1;

	double xoffset = width / 2.0 * (1 - xratio);
	double yoffset = height / 2.0 * (1 - yratio);
	window_width = width * xratio;
	window_height = height * yratio;

	glViewport(xoffset, yoffset, window_width, window_height);
	update_resolution();
}

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;
	switch (key) {
		case GLFW_KEY_E: fov += 0.1; break;
		case GLFW_KEY_Q: fov -= 0.1; break;
		case GLFW_KEY_PAGE_UP: pixelation++; update_resolution(); break;
		case GLFW_KEY_PAGE_DOWN: if (pixelation > 1) { pixelation--; update_resolution(); } break;
		case GLFW_KEY_MINUS: wall_height *= 0.75; break;
		case GLFW_KEY_EQUAL: wall_height *= 1.5; break;
	}
}

static void glfw_error_callback(int code, const char *desc) {
	fprintf(stderr, "(GLFW ERR %#010x) %s\n", code, desc);
	exit(1);
}

static void opengl_message_callback(GLenum source, GLenum type, unsigned id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
	fprintf(stderr, "%s %s\n", (type == GL_DEBUG_TYPE_ERROR) ? "(GL ERR):" : "(GL)", message);
}

