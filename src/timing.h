#ifndef TIMING_H
#define TIMING_H

/**
 * timing.h - Frame timing module for consistent 60 FPS game loop
 *
 * Provides frame-based timing with delta time calculation and
 * frame rate limiting. Uses CLOCK_MONOTONIC for precise timing.
 */

typedef struct {
    double last_frame_time;
    double target_frame_duration;
} Timer;

/**
 * Initialize timer with target FPS.
 *
 * @param timer Pointer to timer structure
 * @param target_fps Target frames per second (typically 60)
 */
void timer_init(Timer* timer, int target_fps);

/**
 * Start a new frame (call at beginning of game loop).
 * Records the current time for delta calculation.
 *
 * @param timer Pointer to timer structure
 */
void timer_start_frame(Timer* timer);

/**
 * Get delta time since last frame.
 *
 * @param timer Pointer to timer structure
 * @return Delta time in seconds (capped to avoid giant jumps)
 */
double timer_get_delta(const Timer* timer);

/**
 * Wait for frame to complete.
 * Sleeps for the remaining frame time to maintain target FPS.
 *
 * @param timer Pointer to timer structure
 */
void timer_wait_frame(Timer* timer);

/**
 * Get current monotonic time in seconds.
 *
 * @return Current time in seconds since arbitrary epoch
 */
double timer_get_time(void);

#endif /* TIMING_H */
