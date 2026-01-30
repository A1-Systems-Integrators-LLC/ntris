#include "input.h"
#include <ncurses.h>

void input_init(void) {
    /* Enable non-blocking mode for getch() */
    nodelay(stdscr, TRUE);

    /* Enable special keys (arrow keys, function keys, etc.) */
    keypad(stdscr, TRUE);

    /* Disable echoing of typed characters */
    noecho();

    /* Make cursor invisible */
    curs_set(0);
}

InputAction input_poll(void) {
    int ch = getch();

    /* No key pressed */
    if (ch == ERR) {
        return INPUT_NONE;
    }

    /* Map keys to actions */
    switch (ch) {
        case KEY_LEFT:
            return INPUT_LEFT;

        case KEY_RIGHT:
            return INPUT_RIGHT;

        case KEY_DOWN:
            return INPUT_DOWN;

        case KEY_UP:
            /* Up arrow for rotation (common in Tetris games) */
            return INPUT_ROTATE;

        case ' ':
            /* Spacebar for hard drop */
            return INPUT_HARD_DROP;

        case 'p':
        case 'P':
            return INPUT_PAUSE;

        case 'q':
        case 'Q':
            return INPUT_QUIT;

        case '\n':
        case '\r':
        case KEY_ENTER:
            /* Enter key for starting game from start screen */
            return INPUT_START;

        default:
            /* Unrecognized key - treat as no input */
            return INPUT_NONE;
    }
}

void input_cleanup(void) {
    /* Restore blocking mode */
    nodelay(stdscr, FALSE);

    /* Re-enable echoing */
    echo();

    /* Make cursor visible again */
    curs_set(1);
}
