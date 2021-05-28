
/* Engine.h
 *
 * Engine constants, structs and declarations.
 *
 * TODO: Merge B_WIDTH and BD_WIDTH.
 *       Allow board convert function to return failure.
 */

#ifndef ENGINE_H
#define ENGINE_H

#include <exec/types.h>

#define BD_WIDTH  20 /* Board size */
#define BD_HEIGHT 16

/* Edited board format */
struct editBoard
{
    WORD tiles[BD_HEIGHT][BD_WIDTH];
};

enum {
    T_FLOOR,
    T_WALL,
    T_OBJECT
};

enum {
    F_NORMAL,
    F_FLAGSTONE
};

enum {
    W_SOLID,
    W_DOOR
};

enum {
    O_BOX,
    O_HERO,
    O_SCROLL,
    O_KEY
};

enum {
    R_ENTER,
    R_BLOCK
};

struct field {
    int type;
    union {
        int floor, wall, object;
    } sub;
    int floor;
};

struct board {
    int placedBoxes, allBoxes, keys;
    struct field array[ BD_HEIGHT ][ BD_WIDTH ];
    WORD x, y; /* Hero */
};

#endif /* ENGINE_H */
