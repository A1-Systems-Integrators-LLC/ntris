#include "board.h"
#include <string.h>

/* Initialize board to empty state */
void board_init(Board* board) {
    memset(board->cells, 0, sizeof(board->cells));
}

/* Check if piece collides with board or boundaries */
bool board_check_collision(const Board* board, PieceType type,
                           RotationState rotation, int x, int y) {
    const PieceShape* shape = piece_get_shape(type, rotation);

    /* Check all 4 blocks of the piece */
    for (int i = 0; i < 4; i++) {
        int block_x = x + shape->cells[i][0];
        int block_y = y + shape->cells[i][1];

        /* Check left/right wall collision */
        if (block_x < 0 || block_x >= BOARD_WIDTH) {
            return true;
        }

        /* Check floor collision */
        if (block_y >= BOARD_HEIGHT) {
            return true;
        }

        /* Check ceiling (allow blocks above visible area during spawn) */
        if (block_y < 0) {
            continue;  /* Allow pieces to extend above board during spawn */
        }

        /* Check collision with existing blocks */
        if (board->cells[block_y][block_x] != 0) {
            return true;
        }
    }

    return false;
}

/* Lock piece into board grid */
void board_lock_piece(Board* board, PieceType type,
                      RotationState rotation, int x, int y) {
    const PieceShape* shape = piece_get_shape(type, rotation);
    int color = piece_get_color(type);

    /* Write all 4 blocks to board */
    for (int i = 0; i < 4; i++) {
        int block_x = x + shape->cells[i][0];
        int block_y = y + shape->cells[i][1];

        /* Only lock blocks within board boundaries */
        if (block_x >= 0 && block_x < BOARD_WIDTH &&
            block_y >= 0 && block_y < BOARD_HEIGHT) {
            board->cells[block_y][block_x] = color;
        }
    }
}

/* Clear completed lines and shift rows down */
int board_clear_lines(Board* board) {
    int lines_cleared = 0;

    /* Scan all rows from bottom to top */
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        bool line_full = true;

        /* Check if row is completely filled */
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board->cells[y][x] == 0) {
                line_full = false;
                break;
            }
        }

        /* If row is full, clear it and shift rows down */
        if (line_full) {
            lines_cleared++;

            /* Shift all rows above this one down by one */
            for (int shift_y = y; shift_y > 0; shift_y--) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    board->cells[shift_y][x] = board->cells[shift_y - 1][x];
                }
            }

            /* Clear top row */
            for (int x = 0; x < BOARD_WIDTH; x++) {
                board->cells[0][x] = 0;
            }

            /* Re-check same row (since we shifted down) */
            y++;
        }
    }

    return lines_cleared;
}

/* Get cell value at position */
int board_get_cell(const Board* board, int x, int y) {
    /* Return 0 for out-of-bounds */
    if (x < 0 || x >= BOARD_WIDTH || y < 0 || y >= BOARD_HEIGHT) {
        return 0;
    }

    return board->cells[y][x];
}

/* Check if spawn position is blocked */
bool board_is_spawn_blocked(const Board* board) {
    /* Standard spawn position is top-center (x=3, y=0) for most pieces
     * Check a few cells at the top-center area */
    const int spawn_x = BOARD_WIDTH / 2 - 1;  /* x=4 for 10-wide board */
    const int spawn_y = 0;

    /* Check 4-cell spawn area at top-center */
    for (int x = spawn_x; x < spawn_x + 2; x++) {
        for (int y = spawn_y; y < spawn_y + 2; y++) {
            if (x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT) {
                if (board->cells[y][x] != 0) {
                    return true;
                }
            }
        }
    }

    return false;
}
