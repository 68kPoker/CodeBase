
/* Basic terms */

#ifndef BOARD_H
#define BOARD_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/* Object type */

struct type
{
    BYTE main, sub; /* Main and sub-type   */
    BYTE id;        /* Optional identifier */
    BYTE frame;     /* Animation frame     */
};

/* Single board cell */

struct cell
{
    /* Consists of a floor type and object type */
    struct type floor, obj;
};

/* Game board */

#define BOARD_W 20
#define BOARD_H 16

struct board
{
    /* Consists of 2D-array of cells */
    struct cell cells[BOARD_H][BOARD_W];
};

#endif /* BOARD_H */
