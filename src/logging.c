#include "logging.h"
#include "timer.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

static bool is_initiated = false;
static struct rc_timer *init_timer;
static const char *RC_LOG_FORMAT = "%.4f [%s] %s\n";
static const char *RC_LOG_URGENCY_LABELS[rc_log_urgency_count] = { "DBUG", "INFO", "NOTE", "WARN", "ERRR" };

void rc_log_init(void) {
	init_timer = rc_timer_create();
	is_initiated = true;
	rc_log(RC_LOG_NOTEWORTHY, "Logging system initialized.");
}

void rc_log(enum rc_log_urgency urgency, const char *message, ...) {
	va_list args;
	va_start(args, message);
	rc_log_variadic(urgency, message, args);
	va_end(args);
}

void rc_log_variadic(enum rc_log_urgency urgency, const char *message, va_list args) {

#ifndef RC_DEBUG
	if (urgency == RC_LOG_VERBOSE)
		return;
#endif

	if (!is_initiated)
		return;

	// Determine size of formatted message
	va_list args_copy;
	va_copy(args_copy, args);
	int formatted_message_length = vsnprintf(NULL, 0, message, args_copy) + 1;
	va_end(args_copy);

	// Allocate space and format message
	char *formatted_message = malloc(sizeof *formatted_message * formatted_message_length);
	if (!formatted_message) {
		fputs("Unable to write log, aborting...", stderr);
		exit(1);
	}
	vsprintf(formatted_message, message, args);

	// Print log in RC_LOG_FORMAT format
	double time_since_init = rc_timer_measure(init_timer);
	const char *urgency_label = RC_LOG_URGENCY_LABELS[urgency];
	printf(RC_LOG_FORMAT, time_since_init, urgency_label, formatted_message);
	free(formatted_message);
}

void rc_log_cleanup(void) {
	if (is_initiated) {
		rc_log(RC_LOG_INFO, "Cleaning up logging system...");
		rc_timer_destroy(init_timer);
		is_initiated = false;
	}
}
