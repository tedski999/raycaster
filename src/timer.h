#ifndef RAYCASTER_TIMER_H
#define RAYCASTER_TIMER_H

struct raycaster_timer;

struct raycaster_timer *rc_timer_create();
double rc_timer_measure(const struct raycaster_timer *timer);
double rc_timer_reset(struct raycaster_timer *timer);
void rc_timer_destroy(struct raycaster_timer *timer);

#endif
