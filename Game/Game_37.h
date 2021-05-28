
#ifndef GAME_H
#define GAME_H

#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

enum
{
    FLOOR_KIND,
    WALL_KIND,
    ITEM_KIND,
    OBJECT_KIND
};

enum
{
    NORMAL_FLOOR,
    FLAGSTONE_FLOOR
};

enum
{
    NORMAL_WALL,
    DOOR_WALL
};

enum
{
    HERO_OBJECT,
    BOX_OBJECT
};

enum
{
    KEY_ITEM,
    FRUIT_ITEM
};

struct gameTile
{
    UBYTE kind, type, floor, aux;
};

struct gameBoard
{
    struct gameTile board[BOARD_HEIGHT][BOARD_WIDTH];
    WORD herox, heroy;
};

void setTile(struct gameTile *t, UWORD gfx);
UWORD getGfx(struct gameTile *t);
BOOL convertBoard(struct gameBoard *gb, struct window *w);
BOOL moveHero(struct gameBoard *gb, WORD dx, WORD dy, struct window *w);

#endif /* GAME_H */
