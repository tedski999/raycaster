#ifndef RC_ERROR_H
#define RC_ERROR_H

#ifdef RC_DEBUG
#define RC_ASSERT(assertion) if (!(assertion)) rc_error("Assertion failed: %s:%i\n", __FILE__, __LINE__)
#else
#define RC_ASSERT(assertion) if (!(assertion)) rc_error("Assertion failed")
#endif

void rc_error(const char *message, ...);

#endif
