#ifndef RAYCASTER_ERROR_H
#define RAYCASTER_ERROR_H

#ifdef RC_DEBUG
#define RC_ASSERT(assertion, message) if (!(assertion)) rc_error("Assertion failed: %s:%i - %s\n", __FILE__, __LINE__, message)
#else
#define RC_ASSERT(assertion, message) if (!(assertion)) rc_error("Assertion failed: %s\n", message)
#endif

void rc_error(const char *message, ...);

#endif
