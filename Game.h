
#ifndef GAME_H
#define GAME_H

#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

#define VERSION 2

enum
{
    RESULT_COMPLETED,
    RESULT_EDIT,
    RESULT_PLAY,
    RESULT_DONE,
    RESULT_QUIT,
    RESULT_RESTART,
    RESULT_START,
    RESULT_LOAD
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
    FLAGSTONE_FLOOR
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
    struct Cell board[BOARD_HEIGHT][BOARD_WIDTH];
    struct boardInfo
    {
        WORD heroX, heroY;
        WORD boxes, placed, keys;
        WORD level, points;
    } info;
};


struct boardHeader
{
    ULONG version; /* Game version */
};

struct gameState
{
    struct boardHeader header;
    struct boardInfo info;
};

#endif
