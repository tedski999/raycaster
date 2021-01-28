#include "error.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef RC_DEBUG
#include <execinfo.h>
#define CALLSTACK_BUFFER_SIZE 128
#endif

void rc_error(const char *const message, ...) {
	va_list args;
	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);

#ifdef RC_DEBUG
	void *callstack[CALLSTACK_BUFFER_SIZE];
	int frame_count = backtrace(callstack, CALLSTACK_BUFFER_SIZE);
	char **strs = backtrace_symbols(callstack, frame_count);
	fprintf(stderr, "Callstack (%i frames):\n", frame_count);
	for (int i = 0; i < frame_count; i++)
		fprintf(stderr, "\t%s\n", strs[i]);
	free(strs);
#endif

	exit(1);
}
