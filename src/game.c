#include "game.h"
#include <stdlib.h>
#include <time.h>

/* Constants */
#define SPAWN_X 3           /* Spawn X position (left edge of piece) */
#define SPAWN_Y 0           /* Spawn Y position (top) */
#define LOCK_DELAY 0.5      /* Lock delay in seconds */
#define LINES_PER_LEVEL 10  /* Lines needed to advance level */

/* Wall kick offsets for rotation attempts (Simple Rotation System) */
static const int WALL_KICK_OFFSETS[][2] = {
    {0, 0},   /* No offset */
    {-1, 0},  /* Left 1 */
    {1, 0},   /* Right 1 */
    {0, -1},  /* Up 1 */
    {-2, 0},  /* Left 2 (for I-piece) */
    {2, 0}    /* Right 2 (for I-piece) */
};
#define WALL_KICK_COUNT 6

/* Scoring multipliers for line clears */
static const int LINE_CLEAR_SCORES[] = {
    0,    /* 0 lines */
    100,  /* 1 line */
    300,  /* 2 lines */
    500,  /* 3 lines */
    800   /* 4 lines (Tetris!) */
};

/* Initialize new game (starts at start screen) */
void game_init(Game* game) {
    /* Initialize board */
    board_init(&game->board);

    /* Set initial state to start screen */
    game->state = GAME_STATE_START_SCREEN;

    /* Initialize stats */
    game->score = 0;
    game->level = 1;
    game->lines_cleared = 0;

    /* Initialize timers */
    game->gravity_timer = 0.0;
    game->lock_delay_timer = 0.0;
    game->is_on_ground = false;

    /* Seed random number generator */
    srand((unsigned int)time(NULL));

    /* Generate first two pieces (but don't spawn yet) */
    game->current_piece = (PieceType)(rand() % PIECE_COUNT);
    game->next_piece = (PieceType)(rand() % PIECE_COUNT);
    game->current_rotation = ROT_0;
    game->piece_x = SPAWN_X;
    game->piece_y = SPAWN_Y;
}

/* Set starting level and begin game from start screen */
void game_set_starting_level(Game* game, int level) {
    /* Clamp level to valid range (1-10) */
    if (level < 1) {
        level = 1;
    } else if (level > 10) {
        level = 10;
    }

    /* Set the starting level */
    game->level = level;

    /* Transition from start screen to playing state */
    game->state = GAME_STATE_PLAYING;

    /* Check if spawn position is valid */
    if (board_check_collision(&game->board, game->current_piece,
                              game->current_rotation, game->piece_x, game->piece_y)) {
        game->state = GAME_STATE_GAME_OVER;
    }
}

/* Spawn next piece */
bool game_spawn_piece(Game* game) {
    /* Move next piece to current */
    game->current_piece = game->next_piece;
    game->current_rotation = ROT_0;
    game->piece_x = SPAWN_X;
    game->piece_y = SPAWN_Y;

    /* Generate new next piece */
    game->next_piece = (PieceType)(rand() % PIECE_COUNT);

    /* Reset ground state */
    game->is_on_ground = false;
    game->lock_delay_timer = 0.0;

    /* Check if spawn position is valid */
    if (board_check_collision(&game->board, game->current_piece,
                              game->current_rotation, game->piece_x, game->piece_y)) {
        game->state = GAME_STATE_GAME_OVER;
        return false;
    }

    return true;
}

/* Lock current piece and handle line clearing */
static void lock_and_clear(Game* game) {
    /* Lock piece into board */
    board_lock_piece(&game->board, game->current_piece,
                     game->current_rotation, game->piece_x, game->piece_y);

    /* Clear lines and update score */
    int lines = board_clear_lines(&game->board);
    if (lines > 0) {
        game->lines_cleared += lines;
        game->score += LINE_CLEAR_SCORES[lines] * game->level;

        /* Check for level up */
        game->level = 1 + (game->lines_cleared / LINES_PER_LEVEL);
    }

    /* Spawn next piece */
    game_spawn_piece(game);
}

/* Check if piece is on ground */
static bool is_grounded(const Game* game) {
    return board_check_collision(&game->board, game->current_piece,
                                 game->current_rotation,
                                 game->piece_x, game->piece_y + 1);
}

/* Update game state */
void game_update(Game* game, double delta_time) {
    if (game->state != GAME_STATE_PLAYING) {
        return;
    }

    /* Update gravity timer */
    game->gravity_timer += delta_time;
    double gravity_speed = game_get_gravity_speed(game);

    /* Apply gravity */
    if (game->gravity_timer >= gravity_speed) {
        game->gravity_timer = 0.0;

        /* Try to move piece down */
        if (!board_check_collision(&game->board, game->current_piece,
                                   game->current_rotation,
                                   game->piece_x, game->piece_y + 1)) {
            game->piece_y++;
            game->is_on_ground = false;
            game->lock_delay_timer = 0.0;
        } else {
            game->is_on_ground = true;
        }
    }

    /* Handle lock delay */
    if (game->is_on_ground || is_grounded(game)) {
        game->is_on_ground = true;
        game->lock_delay_timer += delta_time;

        if (game->lock_delay_timer >= LOCK_DELAY) {
            lock_and_clear(game);
        }
    } else {
        game->lock_delay_timer = 0.0;
    }
}

/* Move piece left */
bool game_move_left(Game* game) {
    if (game->state != GAME_STATE_PLAYING) {
        return false;
    }

    int new_x = game->piece_x - 1;
    if (!board_check_collision(&game->board, game->current_piece,
                               game->current_rotation, new_x, game->piece_y)) {
        game->piece_x = new_x;

        /* Reset lock delay if moving off ground */
        if (!is_grounded(game)) {
            game->is_on_ground = false;
            game->lock_delay_timer = 0.0;
        }

        return true;
    }

    return false;
}

/* Move piece right */
bool game_move_right(Game* game) {
    if (game->state != GAME_STATE_PLAYING) {
        return false;
    }

    int new_x = game->piece_x + 1;
    if (!board_check_collision(&game->board, game->current_piece,
                               game->current_rotation, new_x, game->piece_y)) {
        game->piece_x = new_x;

        /* Reset lock delay if moving off ground */
        if (!is_grounded(game)) {
            game->is_on_ground = false;
            game->lock_delay_timer = 0.0;
        }

        return true;
    }

    return false;
}

/* Soft drop (move down, award 1 point) */
bool game_move_down(Game* game) {
    if (game->state != GAME_STATE_PLAYING) {
        return false;
    }

    int new_y = game->piece_y + 1;
    if (!board_check_collision(&game->board, game->current_piece,
                               game->current_rotation, game->piece_x, new_y)) {
        game->piece_y = new_y;
        game->score += 1;  /* Award 1 point for soft drop */
        game->is_on_ground = false;
        game->lock_delay_timer = 0.0;
        return true;
    }

    return false;
}

/* Rotate piece clockwise with wall kicks */
bool game_rotate(Game* game) {
    if (game->state != GAME_STATE_PLAYING) {
        return false;
    }

    RotationState new_rotation = piece_rotate_cw(game->current_rotation);

    /* Try rotation with wall kick offsets */
    for (int i = 0; i < WALL_KICK_COUNT; i++) {
        int test_x = game->piece_x + WALL_KICK_OFFSETS[i][0];
        int test_y = game->piece_y + WALL_KICK_OFFSETS[i][1];

        if (!board_check_collision(&game->board, game->current_piece,
                                   new_rotation, test_x, test_y)) {
            /* Rotation successful */
            game->current_rotation = new_rotation;
            game->piece_x = test_x;
            game->piece_y = test_y;

            /* Reset lock delay if rotating off ground */
            if (!is_grounded(game)) {
                game->is_on_ground = false;
                game->lock_delay_timer = 0.0;
            }

            return true;
        }
    }

    return false;
}

/* Hard drop */
void game_hard_drop(Game* game) {
    if (game->state != GAME_STATE_PLAYING) {
        return;
    }

    /* Move piece down until collision */
    int drop_distance = 0;
    while (!board_check_collision(&game->board, game->current_piece,
                                  game->current_rotation,
                                  game->piece_x, game->piece_y + 1)) {
        game->piece_y++;
        drop_distance++;
    }

    /* Award points for hard drop (2 points per row) */
    game->score += drop_distance * 2;

    /* Lock piece immediately */
    lock_and_clear(game);
}

/* Toggle pause state */
void game_toggle_pause(Game* game) {
    if (game->state == GAME_STATE_PLAYING) {
        game->state = GAME_STATE_PAUSED;
    } else if (game->state == GAME_STATE_PAUSED) {
        game->state = GAME_STATE_PLAYING;
    }
}

/* Check if game is over */
bool game_is_over(const Game* game) {
    return game->state == GAME_STATE_GAME_OVER;
}

/* Check if game is paused */
bool game_is_paused(const Game* game) {
    return game->state == GAME_STATE_PAUSED;
}

/* Get current score */
int game_get_score(const Game* game) {
    return game->score;
}

/* Get current level */
int game_get_level(const Game* game) {
    return game->level;
}

/* Get total lines cleared */
int game_get_lines(const Game* game) {
    return game->lines_cleared;
}

/* Get next piece for preview */
PieceType game_get_next_piece(const Game* game) {
    return game->next_piece;
}

/* Calculate gravity speed based on level */
double game_get_gravity_speed(const Game* game) {
    /* Formula: 0.8 - ((level - 1) * 0.007) seconds per row */
    double speed = 0.8 - ((game->level - 1) * 0.007);

    /* Clamp to minimum speed to prevent negative values at high levels */
    if (speed < 0.05) {
        speed = 0.05;
    }

    return speed;
}
