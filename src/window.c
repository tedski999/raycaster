#include "window.h"
#include "renderer.h"
#include "error.h"
#include <stdlib.h>
#include <stdio.h>
#include <GLFW/glfw3.h>

#define OPENGL_VERSION_MAJOR 3
#define OPENGL_VERSION_MINOR 2

struct raycaster_window {
	GLFWwindow *window;
};

static void rc_glfw_framebuffer_resize_callback(GLFWwindow *window, int width, int height);
static void rc_glfw_error_callback(int code, const char *desc);

static int window_count = 0;

struct raycaster_window *rc_window_create(const char *const title, int width, int height, bool is_resizable, bool is_cursor_disabled) {

	// Start GLFW if this is the first window
	if (window_count++ == 0) {
		glfwSetErrorCallback(rc_glfw_error_callback);
		glfwInit();
	}
	
	// Open window
	glfwWindowHint(GLFW_RESIZABLE, is_resizable);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	struct raycaster_window *window = malloc(sizeof *window);
	RC_ASSERT(window, "raycaster_window memory allocation");
	*window = (struct raycaster_window) {
		glfwCreateWindow(width, height, title, NULL, NULL)
	};

	// Configure window
	rc_window_set_as_context(window);
	glfwSetFramebufferSizeCallback(window->window, rc_glfw_framebuffer_resize_callback);
	if (is_cursor_disabled)
		glfwSetInputMode(window->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSwapInterval(1); // TODO: togglable vsync

	return window;
}

void rc_window_set_renderer(struct raycaster_window *window, void *renderer) {
	int width, height;
	glfwSetWindowUserPointer(window->window, renderer);
	glfwGetFramebufferSize(window->window, &width, &height);
	rc_renderer_set_dimensions(renderer, width, height);
}

void rc_window_set_as_context(struct raycaster_window *window) {
	glfwMakeContextCurrent(window->window);
}

bool rc_window_is_key_down(struct raycaster_window *window, enum input_key key) {
	return (glfwGetKey(window->window, key) == GLFW_PRESS);
}

void rc_window_get_mouse_position(struct raycaster_window *window, double *const x, double *const y) {
	glfwGetCursorPos(window->window, x, y);
}

bool rc_window_should_close(struct raycaster_window *window) {
	return glfwWindowShouldClose(window->window);
}

void rc_window_update(struct raycaster_window *window) {
	glfwPollEvents();
}

void rc_window_render(struct raycaster_window *window) {
	glfwSwapBuffers(window->window);
}

void rc_window_destroy(struct raycaster_window *window) {
	glfwDestroyWindow(window->window);
	free(window);

	// Terminate GLFW if this is the last window
	if (--window_count == 0)
		glfwTerminate();
}

static void rc_glfw_framebuffer_resize_callback(GLFWwindow *window, int width, int height) {
	struct raycaster_renderer *renderer = glfwGetWindowUserPointer(window);
	if (renderer)
		rc_renderer_set_dimensions(renderer, width, height);
}

static void rc_glfw_error_callback(int code, const char *desc) {
	fprintf(stderr, "GLFW (ERR %#010x): %s\n", code, desc);
	exit(1);
}
