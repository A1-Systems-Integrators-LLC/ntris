#define _POSIX_C_SOURCE 199309L

#include "timing.h"
#include <time.h>
#include <stddef.h>

/* Maximum delta time cap to avoid giant jumps (100ms) */
#define MAX_DELTA_TIME 0.1

/* Nanoseconds per second */
#define NSEC_PER_SEC 1000000000L

/**
 * Get current monotonic time in seconds.
 */
double timer_get_time(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / NSEC_PER_SEC;
}

/**
 * Initialize timer with target FPS.
 */
void timer_init(Timer* timer, int target_fps) {
    if (timer == NULL || target_fps <= 0) {
        return;
    }

    timer->target_frame_duration = 1.0 / (double)target_fps;
    timer->last_frame_time = timer_get_time();
}

/**
 * Start a new frame (mark frame start time).
 */
void timer_start_frame(Timer* timer) {
    if (timer == NULL) {
        return;
    }

    timer->last_frame_time = timer_get_time();
}

/**
 * Get delta time since last frame (in seconds).
 * Capped to MAX_DELTA_TIME to avoid giant jumps if frame takes too long.
 */
double timer_get_delta(const Timer* timer) {
    if (timer == NULL) {
        return 0.0;
    }

    double current_time = timer_get_time();
    double delta = current_time - timer->last_frame_time;

    /* Cap delta to avoid giant jumps on slow frames */
    if (delta > MAX_DELTA_TIME) {
        delta = MAX_DELTA_TIME;
    }

    return delta;
}

/**
 * Wait for frame to complete.
 * Sleeps for the remaining frame time to maintain target FPS.
 */
void timer_wait_frame(Timer* timer) {
    if (timer == NULL) {
        return;
    }

    double current_time = timer_get_time();
    double frame_time = current_time - timer->last_frame_time;
    double sleep_time = timer->target_frame_duration - frame_time;

    if (sleep_time > 0.0) {
        struct timespec sleep_spec;
        sleep_spec.tv_sec = (time_t)sleep_time;
        sleep_spec.tv_nsec = (long)((sleep_time - (double)sleep_spec.tv_sec) * NSEC_PER_SEC);

        nanosleep(&sleep_spec, NULL);
    }
}
