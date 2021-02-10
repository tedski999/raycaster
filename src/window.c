#include "window.h"
#include "renderer.h"
#include "input.h"
#include "error.h"
#include "platform.h"
#include <stdlib.h>
#include <stdio.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct raycaster_window {
	GLFWwindow *window;
};

static void rc_glfw_keyboard_input_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void rc_glfw_mouse_input_callback(GLFWwindow *window, int button, int action, int mods);
static void rc_glfw_mouse_movement_callback(GLFWwindow *window, double xpos, double ypos);
static void rc_glfw_framebuffer_resize_callback(GLFWwindow *window, int width, int height);
static void rc_glfw_error_callback(int code, const char *desc);
static enum input_key rc_glfw_convert_keycode(int keycode);
static enum input_button rc_glfw_convert_button(int button);

static int window_count = 0;

struct raycaster_window *rc_window_create(const char *const title, int width, int height, bool is_resizable, bool is_cursor_disabled, bool is_vsync_enabled) {

	// Start GLFW if this is the first window
	if (window_count++ == 0) {
		glfwSetErrorCallback(rc_glfw_error_callback);
		glfwInit();
	}
	
	// Open window
	glfwWindowHint(GLFW_RESIZABLE, is_resizable);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, RC_OPENGL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, RC_OPENGL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	struct raycaster_window *window = malloc(sizeof *window);
	RC_ASSERT(window, "raycaster_window memory allocation");
	*window = (struct raycaster_window) {
		glfwCreateWindow(width, height, title, NULL, NULL)
	};

	// Configure window
	rc_window_set_as_context(window);
	glfwSetKeyCallback(window->window, rc_glfw_keyboard_input_callback);
	glfwSetMouseButtonCallback(window->window, rc_glfw_mouse_input_callback);
	glfwSetCursorPosCallback(window->window, rc_glfw_mouse_movement_callback);
	glfwSetFramebufferSizeCallback(window->window, rc_glfw_framebuffer_resize_callback);
	if (is_cursor_disabled)
		glfwSetInputMode(window->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	rc_window_set_vsync_enabled(window, is_vsync_enabled);

	return window;
}

void rc_window_set_vsync_enabled(struct raycaster_window *window, bool is_vsync_enabled) {
	GLFWwindow *previous_context = glfwGetCurrentContext();
	glfwMakeContextCurrent(window->window);
	glfwSwapInterval(is_vsync_enabled);
	glfwMakeContextCurrent(previous_context);
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

static void rc_glfw_keyboard_input_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	rc_input_set_keyboard_input(rc_glfw_convert_keycode(key), action == GLFW_PRESS || action == GLFW_REPEAT);
}

static void rc_glfw_mouse_input_callback(GLFWwindow *window, int button, int action, int mods) {
	rc_input_set_mouse_input(rc_glfw_convert_button(button), action == GLFW_PRESS);
}

static void rc_glfw_mouse_movement_callback(GLFWwindow *window, double xpos, double ypos) {
	rc_input_set_mouse_position(xpos, ypos);
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

static enum input_key rc_glfw_convert_keycode(int keycode) {

	// Alphanumeric keys
	if (keycode >= GLFW_KEY_A && keycode <= GLFW_KEY_Z)
		return INPUT_KEY_A + (keycode - GLFW_KEY_A);
	if (keycode >= GLFW_KEY_0 && keycode <= GLFW_KEY_9)
		return INPUT_KEY_0 + (keycode - GLFW_KEY_0);

	// Everything else
	switch (keycode) {
		case GLFW_KEY_SPACE:         return INPUT_KEY_SPACE;
		case GLFW_KEY_ENTER:         return INPUT_KEY_ENTER;
		case GLFW_KEY_BACKSPACE:     return INPUT_KEY_BACKSPACE;
		case GLFW_KEY_TAB:           return INPUT_KEY_TAB;
		case GLFW_KEY_ESCAPE:        return INPUT_KEY_ESCAPE;
		case GLFW_KEY_LEFT_SHIFT:    return INPUT_KEY_SHIFT;
		case GLFW_KEY_RIGHT_SHIFT:   return INPUT_KEY_RIGHT_SHIFT;
		case GLFW_KEY_LEFT_CONTROL:  return INPUT_KEY_CONTROL;
		case GLFW_KEY_RIGHT_CONTROL: return INPUT_KEY_RIGHT_CONTROL;
		case GLFW_KEY_LEFT_ALT:      return INPUT_KEY_ALT;
		case GLFW_KEY_RIGHT_ALT:     return INPUT_KEY_RIGHT_ALT;
		case GLFW_KEY_LEFT_BRACKET:  return INPUT_KEY_LEFT_BRACKET;
		case GLFW_KEY_RIGHT_BRACKET: return INPUT_KEY_RIGHT_BRACKET;
		case GLFW_KEY_SEMICOLON:     return INPUT_KEY_SEMICOLON;
		case GLFW_KEY_APOSTROPHE:    return INPUT_KEY_APOSTROPHE;
		case GLFW_KEY_COMMA:         return INPUT_KEY_COMMA;
		case GLFW_KEY_PERIOD:        return INPUT_KEY_PERIOD;
		case GLFW_KEY_SLASH:         return INPUT_KEY_SLASH;
		case GLFW_KEY_BACKSLASH:     return INPUT_KEY_BACKSLASH;
		case GLFW_KEY_MINUS:         return INPUT_KEY_MINUS;
		case GLFW_KEY_EQUAL:         return INPUT_KEY_EQUALS;
		case GLFW_KEY_INSERT:        return INPUT_KEY_INSERT;
		case GLFW_KEY_DELETE:        return INPUT_KEY_DELETE;
		case GLFW_KEY_HOME:          return INPUT_KEY_HOME;
		case GLFW_KEY_END:           return INPUT_KEY_END;
		case GLFW_KEY_PAGE_UP:       return INPUT_KEY_PAGE_UP;
		case GLFW_KEY_PAGE_DOWN:     return INPUT_KEY_PAGE_DOWN;
		case GLFW_KEY_RIGHT:         return INPUT_KEY_RIGHT;
		case GLFW_KEY_LEFT:          return INPUT_KEY_LEFT;
		case GLFW_KEY_DOWN:          return INPUT_KEY_DOWN;
		case GLFW_KEY_UP:            return INPUT_KEY_UP;
		default: return 0;
	}
}

static enum input_button rc_glfw_convert_button(int button) {
	if (button >= GLFW_MOUSE_BUTTON_1 && button <= GLFW_MOUSE_BUTTON_8)
		return INPUT_BUTTON_1 + (button - GLFW_MOUSE_BUTTON_1);
	return 0;
}
