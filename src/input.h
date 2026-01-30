#ifndef INPUT_H
#define INPUT_H

/**
 * Input Module - Non-blocking keyboard input handling
 *
 * Provides ncurses-based keyboard input with non-blocking polling.
 * Maps keyboard keys to game actions for Tetris controls.
 */

typedef enum {
    INPUT_NONE,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_DOWN,
    INPUT_ROTATE,
    INPUT_HARD_DROP,
    INPUT_PAUSE,
    INPUT_QUIT
} InputAction;

/**
 * Initialize input system
 * Configures ncurses for non-blocking input with keypad support
 */
void input_init(void);

/**
 * Poll for keyboard input
 * Returns immediately if no key is pressed (non-blocking)
 * @return InputAction representing the pressed key, or INPUT_NONE if no key
 */
InputAction input_poll(void);

/**
 * Cleanup input system
 * Restores terminal state
 */
void input_cleanup(void);

#endif /* INPUT_H */
