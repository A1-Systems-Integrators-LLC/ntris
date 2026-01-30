#ifndef SOUND_H
#define SOUND_H

/**
 * Sound Module - Terminal bell wrapper for audio feedback
 *
 * Provides simple audio feedback using the terminal bell.
 * No external audio library dependencies required.
 */

/**
 * Play terminal bell sound.
 *
 * Uses ncurses beep() function if available, with fallback
 * to printf("\a") for portability.
 *
 * This function is typically called when lines are cleared
 * to provide audio feedback to the player.
 */
void sound_beep(void);

#endif /* SOUND_H */
