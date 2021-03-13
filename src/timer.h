#ifndef RC_TIMER_H
#define RC_TIMER_H

struct rc_timer;

struct rc_timer *rc_timer_create();
double rc_timer_measure(const struct rc_timer *timer);
double rc_timer_reset(struct rc_timer *timer);
void rc_timer_destroy(struct rc_timer *timer);

#endif
