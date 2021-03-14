#ifndef RC_LOGGING_H
#define RC_LOGGING_H

#include <stdarg.h>

enum rc_log_urgency {
	RC_LOG_VERBOSE = 0,
	RC_LOG_INFO,
	RC_LOG_NOTEWORTHY,
	RC_LOG_WARN,
	RC_LOG_ERROR,
	rc_log_urgency_count
};

void rc_log_init(void);
void rc_log(enum rc_log_urgency urgency, const char *message, ...);
void rc_log_variadic(enum rc_log_urgency urgency, const char *message, va_list args);
void rc_log_cleanup(void);

#endif
