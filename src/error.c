#include "error.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void rc_error(const char *message, ...) {
	va_list args;
	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);
	exit(1);
}
