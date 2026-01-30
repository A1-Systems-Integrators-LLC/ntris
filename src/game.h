#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include "board.h"
#include "piece.h"

/* Game state enumeration */
typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER
} GameState;

/* Game state structure */
typedef struct {
    Board board;
    GameState state;

    /* Current piece state */
    PieceType current_piece;
    RotationState current_rotation;
    int piece_x;
    int piece_y;

    /* Next piece for preview */
    PieceType next_piece;

    /* Game statistics */
    int score;
    int level;
    int lines_cleared;

    /* Timing state */
    double gravity_timer;
    double lock_delay_timer;
    bool is_on_ground;  /* Track if piece is currently grounded */
} Game;

/* Initialize new game with first piece */
void game_init(Game* game);

/* Spawn next piece at top-center (returns false if game over) */
bool game_spawn_piece(Game* game);

/* Update game state with delta time (handles gravity and lock delay) */
void game_update(Game* game, double delta_time);

/* Movement functions (return true if action succeeded) */
bool game_move_left(Game* game);
bool game_move_right(Game* game);
bool game_move_down(Game* game);  /* Soft drop - awards 1 point */

/* Rotation with wall kicks (returns true if succeeded) */
bool game_rotate(Game* game);

/* Hard drop - instantly places piece, awards 2 points/row */
void game_hard_drop(Game* game);

/* State management */
void game_toggle_pause(Game* game);
bool game_is_over(const Game* game);
bool game_is_paused(const Game* game);

/* Getters for game statistics */
int game_get_score(const Game* game);
int game_get_level(const Game* game);
int game_get_lines(const Game* game);
PieceType game_get_next_piece(const Game* game);

/* Get gravity speed in seconds per row (level-dependent) */
double game_get_gravity_speed(const Game* game);

#endif /* GAME_H */
