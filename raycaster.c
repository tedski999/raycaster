#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define OPENGL_IMMEDIATE

#define DEG2RAD(degree) (degree * (3.14152 / 180))
#define RAD2DEG(degree) (degree * (180 / 3.14152))

#define window_title "raycaster"
#define window_aspect 4.0 / 3.0

#define map_width 24
#define map_height 24

#define player_accel 0.01
#define player_drag 1.2
#define player_turn_speed 0.05

struct ray_result {
	double distance;
	int direction;
	int wall;
};

static void render_viewpoint(const int map[map_height][map_width], double x, double y, double r);
static struct ray_result raycast(const int map[map_height][map_width], double x, double y, double a);
static void glfw_framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void glfw_error_callback(int code, const char *desc);

static int window_width = 640;
static int window_height = 480;
static int pixelation = 5;
static double fov = DEG2RAD(60);

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

	// Open window
	glfwSetErrorCallback(glfw_error_callback);
	glfwInit();
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	GLFWwindow *window = glfwCreateWindow(window_width, window_height, window_title, NULL, NULL);

	// Configure window
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
	glfwSetKeyCallback(window, glfw_key_callback);
	glfwSwapInterval(1);

	// Load OpenGL functions
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (!GLAD_GL_VERSION_3_2) {
		fprintf(stderr, "Your system doesn't support OpenGL >= 3.2!\n");
	}
	printf("%s - %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER));
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Update transform matrix to handle the window resolution with its aspect ratio
	glfw_framebuffer_size_callback(window, window_width, window_height);

	// Configure OpenGL
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// Main game loop
	while (!glfwWindowShouldClose(window)) {

		// Update
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			player_vx += cos(player_r) * player_accel;
			player_vy += sin(player_r) * player_accel;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			player_vx -= cos(player_r) * player_accel;
			player_vy -= sin(player_r) * player_accel;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			player_r += player_turn_speed;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			player_r -= player_turn_speed;

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
	}

	glfwTerminate();
}

static void render_viewpoint(const int map[map_height][map_width], double x, double y, double r) {

	// Divide the window up into artificial pixels
	int num_xpixels = window_width / pixelation;
	int num_ypixels = window_height / pixelation;
	double column, column_width = 1.0 / num_xpixels;
	double row = 0.0, row_width = 1.0 / num_ypixels;

	// Floor and ceiling
	double ray_left_rx = cos(r) + sin(r) * fov, ray_left_ry = sin(r) - cos(r) * fov;
	double ray_right_rx = cos(r) - sin(r) * fov, ray_right_ry = sin(r) + cos(r) * fov;
	while (row < 1.0) {
		double row_dist = 0.5 / (row * fov);

		column = -1.0;
		while (column < 1.0) {
			double lerp = (column + 1.0) / 2.0;
			double xtile = x + row_dist * (ray_left_rx + lerp * (ray_right_rx - ray_left_rx));
			double ytile = y + row_dist * (ray_left_ry + lerp * (ray_right_ry - ray_left_ry));
			int checkerboard = ((int)xtile + (int)ytile) % 2;

#ifdef OPENGL_IMMEDIATE
			glBegin(GL_QUADS);
				// Ceiling
				glColor3d(0.0, 0.2, 0.2);
				glVertex2d(column, row);
				glVertex2d(column, row + row_width);
				glVertex2d(column + column_width, row + row_width);
				glVertex2d(column + column_width, row);
				// Floor
				glColor3d(0.5 * checkerboard * row * fov, 0.5 * row * fov, 0.5 * row * fov);
				glVertex2d(column, -row);
				glVertex2d(column, -(row + row_width));
				glVertex2d(column + column_width, -(row + row_width));
				glVertex2d(column + column_width, -row);
			glEnd();
#endif

			column += column_width;
		}

		row += row_width;
	}

	// Walls
	column = -1.0;
	while (column < 1.0) {

		// Find distance from nearest wall to camera plane
		double ray_offset = column * fov;
		double ray_rx = cos(r) - sin(r) * ray_offset;
		double ray_ry = sin(r) + cos(r) * ray_offset;
		struct ray_result hit = raycast(map, x, y, atan2(ray_ry, ray_rx));
		hit.distance *= 1 / sqrt(ray_rx * ray_rx + ray_ry * ray_ry);

		// Draw wall
		double line_height = 0.5 / (hit.distance * fov);
		line_height = (int)(line_height * num_ypixels + 1) / (double)num_ypixels;
#ifdef OPENGL_IMMEDIATE
		glColor3d(
			(0.5 + 0.5 * hit.direction) * (hit.wall != 1) * (hit.wall %= 4) * 1 / hit.distance,
			(0.5 + 0.5 * hit.direction) * (hit.wall % 2)  * (hit.wall %= 5) * 1 / hit.distance,
			(0.5 + 0.5 * hit.direction) * (hit.wall % 3)  * (hit.wall %= 4) * 1 / hit.distance);
		glBegin(GL_QUADS);
			glVertex2d(column, line_height);
			glVertex2d(column, -line_height);
			glVertex2d(column + column_width, -line_height);
			glVertex2d(column + column_width, line_height);
		glEnd();
#endif

		column += column_width;
	}
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

static void glfw_framebuffer_size_callback(GLFWwindow *window, int width, int height) {
	double xratio = window_aspect * height / width;
	double yratio = 1 / xratio;
	if (xratio > 1) xratio = 1;
	if (yratio > 1) yratio = 1;

	double xoffset = width / 2 * (1 - xratio);
	double yoffset = height / 2 * (1 - yratio);
	window_width = width * xratio;
	window_height = height * yratio;

	glViewport(xoffset, yoffset, window_width, window_height);
}

static void glfw_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;
	switch (key) {
		case GLFW_KEY_E: fov += 0.1; break;
		case GLFW_KEY_Q: fov -= 0.1; break;
		case GLFW_KEY_PAGE_UP: pixelation++; break;
		case GLFW_KEY_PAGE_DOWN: if (pixelation > 1) pixelation--; break;
	}
}

static void glfw_error_callback(int code, const char *desc) {
	fprintf(stderr, "(GLFW ERR %#010x) %s\n", code, desc);
	exit(1);
}

