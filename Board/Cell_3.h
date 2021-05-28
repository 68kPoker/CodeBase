
#ifndef CELL_H
#define CELL_H

#include <exec/types.h>

#include "Pos.h"

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

enum
{
    BLOCK, /* Special blocking cell */
    FLOOR, /* Accessible cell */
    OBJECT, /* Solid object */
    ITEM /* Collectable object */
};

/* Blocks */
enum
{
    WALL, /* Normal wall */
    DOOR
};

/* Floors */
enum
{
    NORMAL,
    FLAGSTONE /* Place for boxes */
};

/* Objects */
enum
{
    BOX,
    HERO
};

/* Items */
enum
{
    KEY,
    FRUITS
};

struct Cell
{
    UWORD type : 2; /* Basic type */
    UWORD sub : 4; /* Sub-type (depends on type) */
    UWORD floor : 4; /* Floor beneath object */
};

struct Board
{
    struct Cell cells[BOARD_HEIGHT][BOARD_WIDTH];
    struct Pos heroPos; /* Hero position */
    UWORD boxCount, placedCount; /* Box count */
};

extern BOOL scanBoard(struct Board *b); /* Calc boxes, locate hero etc. */
extern struct Size *getSize(void);

extern BOOL moveHero(struct Board *b, UWORD dir);
extern BOOL pushObject(struct Board *b, struct Cell *pc, WORD offset);

#endif /* CELL_H */
