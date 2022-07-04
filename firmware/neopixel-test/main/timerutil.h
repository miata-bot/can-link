#ifndef TIMER_UTIL_H
#define TIMER_UTIL_H
#include <time.h>

#define MILLION 1000000

void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td);
#endif