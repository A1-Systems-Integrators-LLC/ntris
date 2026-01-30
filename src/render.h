#ifndef RENDER_H
#define RENDER_H

#include <ncurses.h>
#include "game.h"

/* Renderer state structure */
typedef struct {
    WINDOW* game_win;   /* Main game board window */
    WINDOW* stats_win;  /* Stats panel (score, level, lines) */
    WINDOW* next_win;   /* Next piece preview window */
    int color_pairs[8]; /* Background + 7 piece colors */
} Renderer;

/* Initialize ncurses and create windows */
void render_init(Renderer* renderer);

/* Clear screen */
void render_clear(Renderer* renderer);

/* Draw entire game board with current piece */
void render_draw_game(Renderer* renderer, const Game* game);

/* Draw UI panels (score, level, lines, next piece preview) */
void render_draw_stats(Renderer* renderer, const Game* game);

/* Draw start screen with level selection */
void render_draw_start_screen(Renderer* renderer, int selected_level);

/* Draw pause overlay */
void render_draw_pause(Renderer* renderer);

/* Draw game over screen with final score */
void render_draw_game_over(Renderer* renderer, int final_score);

/* Refresh display (call once per frame) */
void render_refresh(Renderer* renderer);

/* Cleanup ncurses */
void render_cleanup(Renderer* renderer);

#endif /* RENDER_H */
