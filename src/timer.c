#include "timer.h"
#include "error.h"
#include "platform.h"
#include <stdlib.h>

#ifdef RC_LINUX
#include <time.h>
#elif defined RC_WINDOWS
#include <windows.h>
#define CLOCK_MONOTONIC 0
struct timespec { long tv_sec, tv_nsec; };
int clock_gettime(int unused, struct timespec *spec) {
	__int64 cur_time;
	GetSystemTimeAsFileTime((FILETIME *)&cur_time);
	cur_time -= 116444736000000000i64; // 1jan1601 to 1jan1970
	spec->tv_sec = cur_time / 10000000i64; // secs
	spec->tv_nsec = cur_time % 10000000i64 * 100; // nsecs
	return 0;
}
#endif

#define NSEC2SEC(nsec) ((nsec) / 1000000000.0)

static double rc_timer_internal_difference(const struct timespec a, const struct timespec b);

struct raycaster_timer {
	struct timespec start_time;
};

struct raycaster_timer *rc_timer_create() {
	struct raycaster_timer *timer = malloc(sizeof *timer);
	RC_ASSERT(timer, "raycaster_timer memory allocation");
	clock_gettime(CLOCK_MONOTONIC, &timer->start_time);
	return timer;
}

double rc_timer_measure(const struct raycaster_timer *timer) {
	struct timespec cur_time;
	clock_gettime(CLOCK_MONOTONIC, &cur_time);
	return rc_timer_internal_difference(cur_time, timer->start_time);
}

double rc_timer_reset(struct raycaster_timer *timer) {
	struct timespec cur_time;
	clock_gettime(CLOCK_MONOTONIC, &cur_time);
	const double time_elapsed = rc_timer_internal_difference(cur_time, timer->start_time);
	timer->start_time = cur_time;
	return time_elapsed;
}

void rc_timer_destroy(struct raycaster_timer *timer) {
	free(timer);
}

static double rc_timer_internal_difference(const struct timespec a, const struct timespec b) {
	return a.tv_sec - b.tv_sec + NSEC2SEC(a.tv_nsec - b.tv_nsec);
}
