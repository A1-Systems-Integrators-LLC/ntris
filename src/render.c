#include "render.h"
#include <string.h>

/* Window layout constants */
#define BOARD_DISPLAY_WIDTH (BOARD_WIDTH * 2)  /* Each cell is 2 chars wide */
#define BOARD_DISPLAY_HEIGHT BOARD_HEIGHT
#define STATS_PANEL_WIDTH 20
#define NEXT_PIECE_HEIGHT 8

/* Initialize ncurses and create windows */
void render_init(Renderer* renderer) {
    /* Initialize ncurses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  /* Hide cursor */

    /* Initialize colors */
    if (has_colors()) {
        start_color();

        /* Define color pairs (1-7 for pieces, matching piece.c colors) */
        init_pair(1, COLOR_CYAN, COLOR_BLACK);    /* I piece */
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);  /* O piece */
        init_pair(3, COLOR_MAGENTA, COLOR_BLACK); /* T piece */
        init_pair(4, COLOR_GREEN, COLOR_BLACK);   /* S piece */
        init_pair(5, COLOR_RED, COLOR_BLACK);     /* Z piece */
        init_pair(6, COLOR_BLUE, COLOR_BLACK);    /* J piece */
        init_pair(7, COLOR_WHITE, COLOR_BLACK);   /* L piece (orange approximated as white) */

        /* Store color pairs */
        for (int i = 0; i < 8; i++) {
            renderer->color_pairs[i] = i;
        }
    }

    /* Calculate window positions (center game board) */
    int start_y = (LINES - BOARD_DISPLAY_HEIGHT - 2) / 2;  /* -2 for borders */
    int start_x = (COLS - BOARD_DISPLAY_WIDTH - STATS_PANEL_WIDTH - 6) / 2;  /* -6 for borders */

    /* Create game board window */
    renderer->game_win = newwin(BOARD_DISPLAY_HEIGHT + 2, BOARD_DISPLAY_WIDTH + 2,
                                start_y, start_x);
    box(renderer->game_win, 0, 0);

    /* Create stats window (to the right of game board) */
    renderer->stats_win = newwin(BOARD_DISPLAY_HEIGHT + 2, STATS_PANEL_WIDTH,
                                 start_y, start_x + BOARD_DISPLAY_WIDTH + 3);
    box(renderer->stats_win, 0, 0);

    /* Create next piece preview window (top of stats panel) */
    renderer->next_win = newwin(NEXT_PIECE_HEIGHT, STATS_PANEL_WIDTH - 2,
                                start_y + 1, start_x + BOARD_DISPLAY_WIDTH + 4);
    box(renderer->next_win, 0, 0);
}

/* Clear screen */
void render_clear(Renderer* renderer) {
    werase(renderer->game_win);
    werase(renderer->stats_win);
    werase(renderer->next_win);
    box(renderer->game_win, 0, 0);
    box(renderer->stats_win, 0, 0);
    box(renderer->next_win, 0, 0);
}

/* Helper function to draw a cell with color */
static void draw_cell(WINDOW* win, int y, int x, int color) {
    if (color > 0 && color <= 7) {
        wattron(win, COLOR_PAIR(color));
        mvwprintw(win, y, x, "██");
        wattroff(win, COLOR_PAIR(color));
    } else {
        /* Empty cell - draw dark blocks */
        mvwprintw(win, y, x, "··");
    }
}

/* Draw entire game board with current piece */
void render_draw_game(Renderer* renderer, const Game* game) {
    /* Draw locked pieces from board */
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            int cell = board_get_cell(&game->board, x, y);
            draw_cell(renderer->game_win, y + 1, x * 2 + 1, cell);
        }
    }

    /* Draw ghost piece if game is playing */
    if (game->state == GAME_STATE_PLAYING) {
        int ghost_y = game_get_ghost_y(game);

        /* Only draw ghost if it's different from current piece position */
        if (ghost_y != game->piece_y) {
            const PieceShape* shape = piece_get_shape(game->current_piece,
                                                       game->current_rotation);
            int color = piece_get_color(game->current_piece);

            for (int i = 0; i < 4; i++) {
                int px = game->piece_x + shape->cells[i][0];
                int py = ghost_y + shape->cells[i][1];

                /* Only draw if within board bounds */
                if (px >= 0 && px < BOARD_WIDTH && py >= 0 && py < BOARD_HEIGHT) {
                    /* Draw ghost with dim color */
                    if (color > 0 && color <= 7) {
                        wattron(renderer->game_win, COLOR_PAIR(color) | A_DIM);
                        mvwprintw(renderer->game_win, py + 1, px * 2 + 1, "[]");
                        wattroff(renderer->game_win, COLOR_PAIR(color) | A_DIM);
                    }
                }
            }
        }
    }

    /* Draw current piece if game is playing */
    if (game->state == GAME_STATE_PLAYING) {
        const PieceShape* shape = piece_get_shape(game->current_piece,
                                                   game->current_rotation);
        int color = piece_get_color(game->current_piece);

        for (int i = 0; i < 4; i++) {
            int px = game->piece_x + shape->cells[i][0];
            int py = game->piece_y + shape->cells[i][1];

            /* Only draw if within board bounds */
            if (px >= 0 && px < BOARD_WIDTH && py >= 0 && py < BOARD_HEIGHT) {
                draw_cell(renderer->game_win, py + 1, px * 2 + 1, color);
            }
        }
    }
}

/* Draw UI panels (score, level, lines, next piece preview) */
void render_draw_stats(Renderer* renderer, const Game* game) {
    /* Draw next piece preview */
    mvwprintw(renderer->next_win, 0, 2, "NEXT");

    if (game->state != GAME_STATE_GAME_OVER) {
        const PieceShape* next_shape = piece_get_shape(game->next_piece, ROT_0);
        int next_color = piece_get_color(game->next_piece);

        /* Center the next piece in preview window */
        int offset_x = 6;  /* Horizontal centering */
        int offset_y = 3;  /* Vertical centering */

        for (int i = 0; i < 4; i++) {
            int px = next_shape->cells[i][0] + offset_x;
            int py = next_shape->cells[i][1] + offset_y;

            if (next_color > 0 && next_color <= 7) {
                wattron(renderer->next_win, COLOR_PAIR(next_color));
                mvwprintw(renderer->next_win, py, px, "██");
                wattroff(renderer->next_win, COLOR_PAIR(next_color));
            }
        }
    }

    /* Draw stats below next piece preview */
    int stats_y = NEXT_PIECE_HEIGHT + 2;
    mvwprintw(renderer->stats_win, stats_y, 2, "SCORE");
    mvwprintw(renderer->stats_win, stats_y + 1, 2, "%d", game->score);

    mvwprintw(renderer->stats_win, stats_y + 3, 2, "HIGH SCORE");
    mvwprintw(renderer->stats_win, stats_y + 4, 2, "%d", game_get_session_high_score(game));

    mvwprintw(renderer->stats_win, stats_y + 6, 2, "LEVEL");
    mvwprintw(renderer->stats_win, stats_y + 7, 2, "%d", game->level);

    mvwprintw(renderer->stats_win, stats_y + 9, 2, "LINES");
    mvwprintw(renderer->stats_win, stats_y + 10, 2, "%d", game->lines_cleared);
}

/* Draw start screen with level selection */
void render_draw_start_screen(Renderer* renderer, int selected_level) {
    int center_y = BOARD_DISPLAY_HEIGHT / 2 - 5;
    int center_x = BOARD_DISPLAY_WIDTH / 2;

    /* Draw title */
    mvwprintw(renderer->game_win, center_y, center_x - 5, "N T R I S");
    mvwprintw(renderer->game_win, center_y + 2, center_x - 9, "NES-style Tetris");

    /* Draw controls */
    mvwprintw(renderer->game_win, center_y + 5, center_x - 9, "CONTROLS");
    mvwprintw(renderer->game_win, center_y + 6, 2, "Arrows: Move/Rotate");
    mvwprintw(renderer->game_win, center_y + 7, 2, "Space:  Hard Drop");
    mvwprintw(renderer->game_win, center_y + 8, 2, "P:      Pause");
    mvwprintw(renderer->game_win, center_y + 9, 2, "Q:      Quit");

    /* Draw level selection */
    mvwprintw(renderer->game_win, center_y + 12, center_x - 9, "SELECT LEVEL (1-10)");

    /* Draw level options with highlight */
    int levels_per_row = 5;
    int level_start_y = center_y + 14;
    for (int level = 1; level <= 10; level++) {
        int row = (level - 1) / levels_per_row;
        int col = (level - 1) % levels_per_row;
        int y = level_start_y + row;
        int x = center_x - 9 + (col * 4);

        if (level == selected_level) {
            /* Highlight selected level */
            wattron(renderer->game_win, A_REVERSE);
            mvwprintw(renderer->game_win, y, x, "[%2d]", level);
            wattroff(renderer->game_win, A_REVERSE);
        } else {
            mvwprintw(renderer->game_win, y, x, " %2d ", level);
        }
    }

    /* Draw start instruction */
    mvwprintw(renderer->game_win, center_y + 17, center_x - 9, "Press ENTER to start");
}

/* Draw pause overlay */
void render_draw_pause(Renderer* renderer) {
    int center_y = BOARD_DISPLAY_HEIGHT / 2;
    int center_x = BOARD_DISPLAY_WIDTH / 2;

    /* Draw pause message centered on game board */
    mvwprintw(renderer->game_win, center_y, center_x - 3, "PAUSED");
    mvwprintw(renderer->game_win, center_y + 2, center_x - 7, "Press P to resume");
}

/* Draw game over screen with final score and high score status */
void render_draw_game_over(Renderer* renderer, const Game* game) {
    int center_y = BOARD_DISPLAY_HEIGHT / 2;
    int center_x = BOARD_DISPLAY_WIDTH / 2;
    int final_score = game_get_score(game);
    int high_score = game_get_session_high_score(game);

    /* Draw game over message centered on game board */
    mvwprintw(renderer->game_win, center_y - 3, center_x - 5, "GAME OVER");

    mvwprintw(renderer->game_win, center_y - 1, center_x - 6, "Final Score:");
    mvwprintw(renderer->game_win, center_y, center_x - 3, "%d", final_score);

    /* Highlight if this is a new session high score */
    if (final_score == high_score && final_score > 0) {
        mvwprintw(renderer->game_win, center_y + 1, center_x - 8, "NEW SESSION HIGH!");
    } else {
        mvwprintw(renderer->game_win, center_y + 1, center_x - 6, "High Score:");
        mvwprintw(renderer->game_win, center_y + 2, center_x - 3, "%d", high_score);
    }

    mvwprintw(renderer->game_win, center_y + 4, center_x - 7, "Press Q to quit");
}

/* Refresh display (call once per frame) */
void render_refresh(Renderer* renderer) {
    wrefresh(renderer->game_win);
    wrefresh(renderer->stats_win);
    wrefresh(renderer->next_win);
    refresh();
}

/* Cleanup ncurses */
void render_cleanup(Renderer* renderer) {
    if (renderer->game_win) {
        delwin(renderer->game_win);
        renderer->game_win = NULL;
    }
    if (renderer->stats_win) {
        delwin(renderer->stats_win);
        renderer->stats_win = NULL;
    }
    if (renderer->next_win) {
        delwin(renderer->next_win);
        renderer->next_win = NULL;
    }
    endwin();
}
