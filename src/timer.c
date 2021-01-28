#include "timer.h"
#include "error.h"
#include <stdlib.h>
#include <time.h>

// TODO: cross-platform high-res timer

#define NSEC2SEC(nsec) ((nsec) / 1000000000.0)

static double rc_timer_internal_difference(struct timespec a, struct timespec b);

struct raycaster_timer {
	struct timespec start_time;
};

struct raycaster_timer *rc_timer_create() {
	struct raycaster_timer *timer = malloc(sizeof *timer);
	RC_ASSERT(timer, "raycaster_timer memory allocation");
	clock_gettime(CLOCK_MONOTONIC, &timer->start_time);
	return timer;
}

double rc_timer_measure(struct raycaster_timer *timer) {
	struct timespec cur_time;
	clock_gettime(CLOCK_MONOTONIC, &cur_time);
	return rc_timer_internal_difference(cur_time, timer->start_time);
}

double rc_timer_reset(struct raycaster_timer *timer) {
	struct timespec cur_time;
	clock_gettime(CLOCK_MONOTONIC, &cur_time);
	double time_elapsed = rc_timer_internal_difference(cur_time, timer->start_time);
	timer->start_time = cur_time;
	return time_elapsed;
}

void rc_timer_destroy(struct raycaster_timer *timer) {
	free(timer);
}

static double rc_timer_internal_difference(struct timespec a, struct timespec b) {
	return a.tv_sec - b.tv_sec + NSEC2SEC(a.tv_nsec - b.tv_nsec);
}
