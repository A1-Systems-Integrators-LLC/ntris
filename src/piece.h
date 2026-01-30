#ifndef PIECE_H
#define PIECE_H

/* Tetromino piece definitions, shapes, colors, and rotation logic */

/* Piece types (7 standard Tetrominos) */
typedef enum {
    PIECE_I,  /* Cyan straight piece */
    PIECE_O,  /* Yellow square piece */
    PIECE_T,  /* Purple T-piece */
    PIECE_S,  /* Green S-piece */
    PIECE_Z,  /* Red Z-piece */
    PIECE_J,  /* Blue J-piece */
    PIECE_L,  /* Orange L-piece */
    PIECE_COUNT
} PieceType;

/* Rotation states (clockwise from spawn orientation) */
typedef enum {
    ROT_0 = 0,    /* 0 degrees (spawn orientation) */
    ROT_90 = 1,   /* 90 degrees clockwise */
    ROT_180 = 2,  /* 180 degrees */
    ROT_270 = 3   /* 270 degrees clockwise */
} RotationState;

/* Piece shape representation (4 blocks with x,y coordinates) */
typedef struct {
    int cells[4][2];  /* [block_index][x/y] - 4 blocks, each with x,y coordinate */
} PieceShape;

/* Get piece shape for given type and rotation state */
const PieceShape* piece_get_shape(PieceType type, RotationState rotation);

/* Get ncurses color pair for piece type (1-7) */
int piece_get_color(PieceType type);

/* Get next rotation state clockwise (with wrap-around) */
RotationState piece_rotate_cw(RotationState current);

/* Get piece name string for display */
const char* piece_get_name(PieceType type);

#endif /* PIECE_H */
