/**
 * Unit Tests for Sound Module
 *
 * Basic smoke tests to verify sound_beep() can be called
 * without crashing and behaves correctly.
 */

#include "../src/sound.h"
#include <stdio.h>
#include <ncurses.h>

/**
 * Test that sound_beep() can be called without crashing.
 *
 * This is a smoke test - we can't easily verify the actual
 * audio output in an automated test, but we can verify the
 * function executes without errors.
 */
int test_sound_beep_no_crash(void) {
    printf("TEST: sound_beep() executes without crash...\n");

    /* Initialize ncurses to ensure beep() will work */
    initscr();

    /* Call sound_beep() - should not crash */
    sound_beep();

    /* Clean up ncurses */
    endwin();

    printf("  PASS: sound_beep() executed successfully\n");
    return 0;
}

/**
 * Test that sound_beep() works when ncurses is not initialized.
 * Should fall back to printf("\a") gracefully.
 */
int test_sound_beep_fallback(void) {
    printf("TEST: sound_beep() fallback (no ncurses init)...\n");

    /* Call sound_beep() without initializing ncurses */
    /* This should trigger the fallback to printf("\a") */
    sound_beep();

    printf("  PASS: sound_beep() fallback executed successfully\n");
    return 0;
}

/**
 * Test that sound_beep() can be called multiple times.
 */
int test_sound_beep_multiple_calls(void) {
    printf("TEST: sound_beep() multiple calls...\n");

    initscr();

    /* Call multiple times - should work without issues */
    sound_beep();
    sound_beep();
    sound_beep();

    endwin();

    printf("  PASS: multiple sound_beep() calls succeeded\n");
    return 0;
}

int main(void) {
    int failures = 0;

    printf("Running Sound Module Tests\n");
    printf("===========================\n\n");

    /* Run all tests */
    failures += test_sound_beep_no_crash();
    failures += test_sound_beep_fallback();
    failures += test_sound_beep_multiple_calls();

    /* Summary */
    printf("\n===========================\n");
    if (failures == 0) {
        printf("All tests PASSED\n");
        return 0;
    } else {
        printf("%d test(s) FAILED\n", failures);
        return 1;
    }
}
