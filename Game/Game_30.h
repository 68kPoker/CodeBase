
#ifndef GAME_H
#define GAME_H

#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

#define VERSION 3

enum
{
    RESULT_COMPLETED,
    RESULT_EDIT,
    RESULT_PLAY,
    RESULT_DONE,
    RESULT_QUIT,
    RESULT_RESTART,
    RESULT_START,
    RESULT_LOAD,
    RESULT_SELECT
};

enum
{
    STATE_LOADED,
    LEVEL_LOADED,
    LOAD_FAILURE
};

enum
{
    FLOOR_KIND,
    WALL_KIND,
    OBJECT_KIND,
    ITEM_KIND
};

enum
{
    NORMAL_FLOOR,
    FLAGSTONE_FLOOR,
    MUD_FLOOR,
    FILLED_FLOOR
};

enum
{
    NORMAL_WALL,
    DOOR_WALL
};

enum
{
    BOX_OBJECT,
    HERO_OBJECT,
    PLACED_OBJECT
};

enum
{
    FRUIT_ITEM,
    KEY_ITEM
};

struct Cell
{
    WORD kind:  4;
    WORD subKind: 4;
    WORD floor: 4;
};

struct Board
{
    ULONG version; /* Game version */
    struct Cell board[BOARD_HEIGHT][BOARD_WIDTH];
};

struct gameState
{
    ULONG version; /* Game version */
    /* Player inventory, points etc. */
    WORD heroX, heroY, curDir;
    WORD boxes, placed, keys;
    WORD level, points;
    struct Board board; /* Current board state */
    WORD maxLevel, totalPoints;
};

#endif
