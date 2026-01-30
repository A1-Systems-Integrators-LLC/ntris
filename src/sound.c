#include "sound.h"
#include <ncurses.h>
#include <stdio.h>

void sound_beep(void) {
    /* Attempt to use ncurses beep() function */
    int result = beep();

    /* Fallback to printf if beep() fails or is not supported */
    if (result == ERR) {
        printf("\a");
        fflush(stdout);
    }
}
