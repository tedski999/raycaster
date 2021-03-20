#include "window.h"
#include "logging.h"
#include "error.h"
#include "renderer.h"
#include "input.h"
#include "platform.h"
#include <stdlib.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct rc_window {
	GLFWwindow *window;
};

static void rc_glfw_keyboard_input_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
static void rc_glfw_mouse_input_callback(GLFWwindow *window, int button, int action, int mods);
static void rc_glfw_mouse_movement_callback(GLFWwindow *window, double xpos, double ypos);
static void rc_glfw_framebuffer_resize_callback(GLFWwindow *window, int width, int height);
static void rc_glfw_error_callback(int code, const char *desc);
static enum rc_input_key rc_glfw_convert_keycode(int keycode);
static enum rc_input_button rc_glfw_convert_button(int button);

static int window_count = 0;

struct rc_window *rc_window_create(const char *title, int width, int height, bool is_resizable, bool is_cursor_disabled, bool is_vsync_enabled) {

	// Start GLFW if this is the first window
	if (window_count++ == 0) {
		rc_log(RC_LOG_NOTEWORTHY, "Initializing GLFW...");
		glfwSetErrorCallback(rc_glfw_error_callback);
		glfwInit();
	}

	rc_log(RC_LOG_INFO, "Opening new window...");
	
	// Open window
	glfwWindowHint(GLFW_RESIZABLE, is_resizable);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, RC_OPENGL_VERSION_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, RC_OPENGL_VERSION_MINOR);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	struct rc_window *window = malloc(sizeof *window);
	RC_ASSERT(window);
	*window = (struct rc_window) {
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

	// Update mouse position to be relative to the new window
	rc_window_update(window);
	rc_input_update();

	return window;
}

void rc_window_set_vsync_enabled(const struct rc_window *window, bool is_vsync_enabled) {
	rc_log(RC_LOG_INFO, (is_vsync_enabled) ? "Enabling v-sync..." : "Disabling v-sync...");
	GLFWwindow *previous_context = glfwGetCurrentContext();
	glfwMakeContextCurrent(window->window);
	glfwSwapInterval(is_vsync_enabled);
	glfwMakeContextCurrent(previous_context);
}

void rc_window_set_renderer(const struct rc_window *window, const struct rc_renderer *renderer) {
	rc_log(RC_LOG_INFO, "Updating window renderer...");
	int width, height;
	glfwSetWindowUserPointer(window->window, (void *)renderer);
	glfwGetFramebufferSize(window->window, &width, &height);
	rc_renderer_set_dimensions(renderer, width, height);
}

void rc_window_set_as_context(const struct rc_window *window) {
	glfwMakeContextCurrent(window->window);
}

bool rc_window_should_close(const struct rc_window *window) {
	return glfwWindowShouldClose(window->window);
}

void rc_window_update(const struct rc_window *window) {
	glfwPollEvents();
}

void rc_window_render(const struct rc_window *window) {
	glfwSwapBuffers(window->window);
}

void rc_window_destroy(struct rc_window *window) {
	rc_log(RC_LOG_INFO, "Closing window...");
	glfwDestroyWindow(window->window);
	free(window);

	// Terminate GLFW if this is the last window
	if (--window_count == 0) {
		rc_log(RC_LOG_NOTEWORTHY, "Terminating GLFW...");
		glfwTerminate();
	}
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
	rc_log(RC_LOG_INFO, "GLFW framebuffer resize...");
	struct rc_renderer *renderer = glfwGetWindowUserPointer(window);
	if (renderer)
		rc_renderer_set_dimensions(renderer, width, height);
}

static void rc_glfw_error_callback(int code, const char *desc) {
	rc_error("GLFW (ERR %#010x): %s\n", code, desc);
}

static enum rc_input_key rc_glfw_convert_keycode(int keycode) {

	// Alphanumeric keys
	if (keycode >= GLFW_KEY_A && keycode <= GLFW_KEY_Z)
		return RC_INPUT_KEY_A + (keycode - GLFW_KEY_A);
	if (keycode >= GLFW_KEY_0 && keycode <= GLFW_KEY_9)
		return RC_INPUT_KEY_0 + (keycode - GLFW_KEY_0);

	// Everything else
	switch (keycode) {
		case GLFW_KEY_SPACE:         return RC_INPUT_KEY_SPACE;
		case GLFW_KEY_ENTER:         return RC_INPUT_KEY_ENTER;
		case GLFW_KEY_BACKSPACE:     return RC_INPUT_KEY_BACKSPACE;
		case GLFW_KEY_TAB:           return RC_INPUT_KEY_TAB;
		case GLFW_KEY_ESCAPE:        return RC_INPUT_KEY_ESCAPE;
		case GLFW_KEY_LEFT_SHIFT:    return RC_INPUT_KEY_SHIFT;
		case GLFW_KEY_RIGHT_SHIFT:   return RC_INPUT_KEY_RIGHT_SHIFT;
		case GLFW_KEY_LEFT_CONTROL:  return RC_INPUT_KEY_CONTROL;
		case GLFW_KEY_RIGHT_CONTROL: return RC_INPUT_KEY_RIGHT_CONTROL;
		case GLFW_KEY_LEFT_ALT:      return RC_INPUT_KEY_ALT;
		case GLFW_KEY_RIGHT_ALT:     return RC_INPUT_KEY_RIGHT_ALT;
		case GLFW_KEY_LEFT_BRACKET:  return RC_INPUT_KEY_LEFT_BRACKET;
		case GLFW_KEY_RIGHT_BRACKET: return RC_INPUT_KEY_RIGHT_BRACKET;
		case GLFW_KEY_SEMICOLON:     return RC_INPUT_KEY_SEMICOLON;
		case GLFW_KEY_APOSTROPHE:    return RC_INPUT_KEY_APOSTROPHE;
		case GLFW_KEY_COMMA:         return RC_INPUT_KEY_COMMA;
		case GLFW_KEY_PERIOD:        return RC_INPUT_KEY_PERIOD;
		case GLFW_KEY_SLASH:         return RC_INPUT_KEY_SLASH;
		case GLFW_KEY_BACKSLASH:     return RC_INPUT_KEY_BACKSLASH;
		case GLFW_KEY_MINUS:         return RC_INPUT_KEY_MINUS;
		case GLFW_KEY_EQUAL:         return RC_INPUT_KEY_EQUALS;
		case GLFW_KEY_INSERT:        return RC_INPUT_KEY_INSERT;
		case GLFW_KEY_DELETE:        return RC_INPUT_KEY_DELETE;
		case GLFW_KEY_HOME:          return RC_INPUT_KEY_HOME;
		case GLFW_KEY_END:           return RC_INPUT_KEY_END;
		case GLFW_KEY_PAGE_UP:       return RC_INPUT_KEY_PAGE_UP;
		case GLFW_KEY_PAGE_DOWN:     return RC_INPUT_KEY_PAGE_DOWN;
		case GLFW_KEY_RIGHT:         return RC_INPUT_KEY_RIGHT;
		case GLFW_KEY_LEFT:          return RC_INPUT_KEY_LEFT;
		case GLFW_KEY_DOWN:          return RC_INPUT_KEY_DOWN;
		case GLFW_KEY_UP:            return RC_INPUT_KEY_UP;
			rc_log(RC_LOG_WARN, "Unrecognized key input!");
			default: return 0;
	}
}

static enum rc_input_button rc_glfw_convert_button(int button) {
	if (button >= GLFW_MOUSE_BUTTON_1 && button <= GLFW_MOUSE_BUTTON_8)
		return RC_INPUT_BUTTON_1 + (button - GLFW_MOUSE_BUTTON_1);
	rc_log(RC_LOG_WARN, "Unrecognized mouse button input!");
	return 0;
}
