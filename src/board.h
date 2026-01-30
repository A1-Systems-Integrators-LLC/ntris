#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include "piece.h"

/* Game board dimensions */
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

/* Board grid structure (10x20 cells) */
typedef struct {
    int cells[BOARD_HEIGHT][BOARD_WIDTH];  /* 0 = empty, 1-7 = piece color */
} Board;

/* Initialize board to empty state (all cells = 0) */
void board_init(Board* board);

/* Check if piece would collide with board or boundaries
 * Returns true if collision detected, false if position is valid */
bool board_check_collision(const Board* board, PieceType type,
                           RotationState rotation, int x, int y);

/* Lock piece into board grid at given position
 * Writes piece color to all 4 block positions */
void board_lock_piece(Board* board, PieceType type,
                      RotationState rotation, int x, int y);

/* Clear completed lines and shift rows down
 * Returns number of lines cleared (0-4) */
int board_clear_lines(Board* board);

/* Get cell value at position (for rendering)
 * Returns 0 if empty, 1-7 for piece color
 * Returns 0 for out-of-bounds coordinates */
int board_get_cell(const Board* board, int x, int y);

/* Check if spawn position (top-center) is blocked
 * Returns true if spawn position overlaps existing blocks */
bool board_is_spawn_blocked(const Board* board);

#endif /* BOARD_H */
