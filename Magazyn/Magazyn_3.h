
#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include "Board.h"

#define ABS(a) ((a)>=0?(a):-(a))

/* Floor type list */

enum
{
    FLOOR_NONE,
    FLOOR_NORMAL,
    FLOOR_FLAGSTONE
};

/* Object type list */

enum
{
    OBJECT_NONE,
    OBJECT_WALL,
    OBJECT_BOX,
    OBJECT_ITEM,
    OBJECT_DOOR,
    OBJECT_HERO
};

/* Item type list */

enum
{
    ITEM_KEY,
    ITEM_FRUIT
};

/* Enhanced board */

struct warehouse
{
    struct board board[2]; /* Double-buffered */
    BYTE buffer;
    BYTE boxes; /* Amount of boxes on the board */
    BYTE placed; /* Amount of placed boxes */
    BYTE keys;
    BYTE herox, heroy;
};

struct warehouse *initWarehouse(WORD width, WORD height);
LONG play(struct warehouse *wh);
void freeWarehouse(struct warehouse *wh);

#endif /* WAREHOUSE_H */
