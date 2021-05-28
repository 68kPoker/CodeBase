
#ifndef GAME_H
#define GAME_H

#include "Engine.h"
#include "IFF.h"
#include "System.h"

enum
{
    TILE_FLOOR,
    TILE_WALL,
    TILE_BOX,
    TILE_PLACE,
    TILE_HERO
};

typedef struct gameData GAMEDATA;
typedef struct gameInfo GAMEINFO;

struct gameData
{
    GFX *graphics;
    Class *cl; /* My image class */
    Object *img[IID_COUNT];

    struct sysData syspart;
};

/* Run-time system variables */
struct gameInfo
{
    GFX *gfx;

    struct board gameBoard;
    struct editBoard editBoard;

    /* Previous cursor position */
    /* Paint mode. */
    /* Current tile. */
    /* Game mode */

    WORD prevx, prevy;
    BOOL paint;
    WORD tile;
    BOOL game;
};

#endif /* GAME_H */
