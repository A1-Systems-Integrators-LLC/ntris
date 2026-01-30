#include "piece.h"
#include <stddef.h>

/*
 * Tetromino rotation tables using 4x4 coordinate grids (Standard Rotation System - SRS)
 *
 * Each piece has 4 rotation states (0°, 90°, 180°, 270°)
 * Coordinates are relative to a reference point (center of rotation)
 * Format: {x, y} where x is horizontal (right +), y is vertical (down +)
 */

/* I-piece (Cyan) - Straight line piece */
static const PieceShape I_SHAPES[4] = {
    /* ROT_0: Horizontal #### */
    {{{0, 1}, {1, 1}, {2, 1}, {3, 1}}},
    /* ROT_90: Vertical */
    {{{2, 0}, {2, 1}, {2, 2}, {2, 3}}},
    /* ROT_180: Horizontal */
    {{{0, 2}, {1, 2}, {2, 2}, {3, 2}}},
    /* ROT_270: Vertical */
    {{{1, 0}, {1, 1}, {1, 2}, {1, 3}}}
};

/* O-piece (Yellow) - Square piece (no rotation needed, but all states same) */
static const PieceShape O_SHAPES[4] = {
    /* ROT_0: Square ## */
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}},
    /* ROT_90: Same as ROT_0 */
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}},
    /* ROT_180: Same as ROT_0 */
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}},
    /* ROT_270: Same as ROT_0 */
    {{{1, 0}, {2, 0}, {1, 1}, {2, 1}}}
};

/* T-piece (Purple) - T-shaped piece */
static const PieceShape T_SHAPES[4] = {
    /* ROT_0: T-shape  #  */
    /*               ### */
    {{{1, 0}, {0, 1}, {1, 1}, {2, 1}}},
    /* ROT_90: T rotated right */
    {{{1, 0}, {1, 1}, {2, 1}, {1, 2}}},
    /* ROT_180: T upside down */
    {{{0, 1}, {1, 1}, {2, 1}, {1, 2}}},
    /* ROT_270: T rotated left */
    {{{1, 0}, {0, 1}, {1, 1}, {1, 2}}}
};

/* S-piece (Green) - S-shaped piece */
static const PieceShape S_SHAPES[4] = {
    /* ROT_0: S-shape  ## */
    /*               ##  */
    {{{1, 0}, {2, 0}, {0, 1}, {1, 1}}},
    /* ROT_90: S vertical */
    {{{1, 0}, {1, 1}, {2, 1}, {2, 2}}},
    /* ROT_180: S horizontal */
    {{{1, 1}, {2, 1}, {0, 2}, {1, 2}}},
    /* ROT_270: S vertical */
    {{{0, 0}, {0, 1}, {1, 1}, {1, 2}}}
};

/* Z-piece (Red) - Z-shaped piece */
static const PieceShape Z_SHAPES[4] = {
    /* ROT_0: Z-shape ##  */
    /*                ## */
    {{{0, 0}, {1, 0}, {1, 1}, {2, 1}}},
    /* ROT_90: Z vertical */
    {{{2, 0}, {1, 1}, {2, 1}, {1, 2}}},
    /* ROT_180: Z horizontal */
    {{{0, 1}, {1, 1}, {1, 2}, {2, 2}}},
    /* ROT_270: Z vertical */
    {{{1, 0}, {0, 1}, {1, 1}, {0, 2}}}
};

/* J-piece (Blue) - J-shaped piece */
static const PieceShape J_SHAPES[4] = {
    /* ROT_0: J-shape #   */
    /*               ### */
    {{{0, 0}, {0, 1}, {1, 1}, {2, 1}}},
    /* ROT_90: J rotated right */
    {{{1, 0}, {2, 0}, {1, 1}, {1, 2}}},
    /* ROT_180: J upside down */
    {{{0, 1}, {1, 1}, {2, 1}, {2, 2}}},
    /* ROT_270: J rotated left */
    {{{1, 0}, {1, 1}, {0, 2}, {1, 2}}}
};

/* L-piece (Orange) - L-shaped piece */
static const PieceShape L_SHAPES[4] = {
    /* ROT_0: L-shape   # */
    /*               ### */
    {{{2, 0}, {0, 1}, {1, 1}, {2, 1}}},
    /* ROT_90: L rotated right */
    {{{1, 0}, {1, 1}, {1, 2}, {2, 2}}},
    /* ROT_180: L upside down */
    {{{0, 1}, {1, 1}, {2, 1}, {0, 2}}},
    /* ROT_270: L rotated left */
    {{{0, 0}, {1, 0}, {1, 1}, {1, 2}}}
};

/* Master rotation table indexed by [piece_type][rotation_state] */
static const PieceShape* const ROTATION_TABLES[PIECE_COUNT] = {
    I_SHAPES,
    O_SHAPES,
    T_SHAPES,
    S_SHAPES,
    Z_SHAPES,
    J_SHAPES,
    L_SHAPES
};

/* Color mappings for each piece type (ncurses color pairs 1-7) */
static const int PIECE_COLORS[PIECE_COUNT] = {
    1,  /* PIECE_I: Cyan */
    2,  /* PIECE_O: Yellow */
    3,  /* PIECE_T: Purple */
    4,  /* PIECE_S: Green */
    5,  /* PIECE_Z: Red */
    6,  /* PIECE_J: Blue */
    7   /* PIECE_L: Orange */
};

/* Piece name strings for display */
static const char* const PIECE_NAMES[PIECE_COUNT] = {
    "I-piece",
    "O-piece",
    "T-piece",
    "S-piece",
    "Z-piece",
    "J-piece",
    "L-piece"
};

/* Public API implementations */

const PieceShape* piece_get_shape(PieceType type, RotationState rotation) {
    if (type < 0 || type >= PIECE_COUNT) {
        return NULL;
    }
    if (rotation < ROT_0 || rotation > ROT_270) {
        return NULL;
    }
    return &ROTATION_TABLES[type][rotation];
}

int piece_get_color(PieceType type) {
    if (type < 0 || type >= PIECE_COUNT) {
        return 0;  /* Return 0 for invalid piece */
    }
    return PIECE_COLORS[type];
}

RotationState piece_rotate_cw(RotationState current) {
    /* Wrap around from ROT_270 to ROT_0 */
    return (RotationState)((current + 1) % 4);
}

const char* piece_get_name(PieceType type) {
    if (type < 0 || type >= PIECE_COUNT) {
        return "Invalid";
    }
    return PIECE_NAMES[type];
}
