
#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

#define VERSION 1

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
    HERO_OBJECT
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

struct boardHeader
{
    ULONG version; /* Game version */
    WORD level; /* Level number */
};

struct Board
{
    struct Cell board[BOARD_HEIGHT][BOARD_WIDTH];
};
