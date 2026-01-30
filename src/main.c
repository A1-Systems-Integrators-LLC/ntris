#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <locale.h>
#include "timing.h"
#include "input.h"
#include "game.h"
#include "render.h"
#include "sound.h"

/**
 * main.c - Main entry point and game loop orchestration
 *
 * Coordinates all modules to run the Tetris game:
 * - Initializes timing, rendering, input, and game systems
 * - Runs main game loop at 60 FPS
 * - Processes input and updates game state
 * - Renders game visuals
 * - Handles pause and game over states
 * - Cleans up on exit
 */

/**
 * Handle input action by calling appropriate game function
 */
static void handle_input(Game* game, InputAction action, bool* should_quit, int* selected_level) {
    switch (action) {
        case INPUT_LEFT:
            if (game->state == GAME_STATE_START_SCREEN) {
                /* Navigate level selection left */
                (*selected_level)--;
                if (*selected_level < 1) {
                    *selected_level = 10;  /* Wrap around */
                }
            } else {
                game_move_left(game);
            }
            break;
        case INPUT_RIGHT:
            if (game->state == GAME_STATE_START_SCREEN) {
                /* Navigate level selection right */
                (*selected_level)++;
                if (*selected_level > 10) {
                    *selected_level = 1;  /* Wrap around */
                }
            } else {
                game_move_right(game);
            }
            break;
        case INPUT_DOWN:
            if (game->state == GAME_STATE_START_SCREEN) {
                /* Navigate level selection down (next row) */
                (*selected_level) += 5;
                if (*selected_level > 10) {
                    *selected_level = (*selected_level % 10);
                    if (*selected_level == 0) {
                        *selected_level = 10;
                    }
                }
            } else {
                game_move_down(game);
            }
            break;
        case INPUT_ROTATE:
            if (game->state == GAME_STATE_START_SCREEN) {
                /* Navigate level selection up (previous row) */
                (*selected_level) -= 5;
                if (*selected_level < 1) {
                    *selected_level = *selected_level + 10;
                }
            } else {
                game_rotate(game);
            }
            break;
        case INPUT_HARD_DROP:
            if (game->state != GAME_STATE_START_SCREEN) {
                game_hard_drop(game);
            }
            break;
        case INPUT_PAUSE:
            if (game->state != GAME_STATE_START_SCREEN) {
                game_toggle_pause(game);
            }
            break;
        case INPUT_QUIT:
            *should_quit = true;
            break;
        case INPUT_START:
            if (game->state == GAME_STATE_START_SCREEN) {
                /* Start game with selected level */
                game_set_starting_level(game, *selected_level);
            }
            break;
        case INPUT_NONE:
            /* No input this frame */
            break;
    }
}

/**
 * Main entry point
 */
int main(int argc, char** argv) {
    /* Initialize locale for UTF-8 support */
    setlocale(LC_ALL, "");

    /* Handle --version flag */
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--version") == 0) {
                printf("ntris version 1.0\n");
                printf("NES-style Tetris clone for Linux terminal\n");
                return EXIT_SUCCESS;
            }
        }
    }

    /* Initialize all modules */
    Timer timer;
    Renderer renderer;
    Game game;

    timer_init(&timer, 60);  /* 60 FPS target */
    render_init(&renderer);
    input_init();
    game_init(&game);

    /* Game loop control */
    bool should_quit = false;
    int selected_level = 1;  /* Default starting level */

    /* Main game loop - runs until quit requested */
    while (!should_quit) {
        /* Compute delta time BEFORE resetting frame timer */
        double delta = timer_get_delta(&timer);
        timer_start_frame(&timer);

        /* INPUT PHASE: Poll keyboard and map to game actions */
        InputAction action = input_poll();
        handle_input(&game, action, &should_quit, &selected_level);

        /* UPDATE PHASE: Update game state with delta time */
        if (!game_is_paused(&game) && game.state != GAME_STATE_START_SCREEN) {
            game_update(&game, delta);
        }

        /* RENDER PHASE: Clear → Draw game → Draw stats → Draw overlays → Refresh */
        render_clear(&renderer);

        if (game.state == GAME_STATE_START_SCREEN) {
            /* Draw start screen with level selection */
            render_draw_start_screen(&renderer, selected_level);
        } else {
            /* Draw normal game */
            render_draw_game(&renderer, &game);
            render_draw_stats(&renderer, &game);

            /* Draw pause overlay if paused */
            if (game_is_paused(&game)) {
                render_draw_pause(&renderer);
            }

            /* Draw game over screen if game ended */
            if (game_is_over(&game)) {
                render_draw_game_over(&renderer, &game);
            }
        }

        render_refresh(&renderer);

        /* TIMING PHASE: Wait for remaining frame time to maintain 60 FPS */
        timer_wait_frame(&timer);
    }

    /* Cleanup all modules on exit */
    render_cleanup(&renderer);
    input_cleanup();

    return EXIT_SUCCESS;
}
