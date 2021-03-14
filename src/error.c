#include "error.h"
#include "logging.h"
#include <stdlib.h>
#include <stdarg.h>

void rc_error(const char *message, ...) {
	va_list args;
	va_start(args, message);
	rc_log_variadic(RC_LOG_ERROR, message, args);
	va_end(args);
	exit(1);
}
