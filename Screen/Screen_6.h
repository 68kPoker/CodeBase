
#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

enum
{
    WALL,
    FLOOR,
    HERO,
    BOX,
    FRUIT,
    FLAGSTONE
};

struct copper
{
    struct ViewPort *viewPort;
    WORD signal;
    struct Task *task;
};

struct tile
{
    BYTE type, counter;
    BYTE flags, id;
};

struct board
{
    struct tile tiles[BOARD_HEIGHT][BOARD_WIDTH];
};

BOOL initScreen(WORD w, WORD h, UBYTE nPlanes, struct IFFHandle *iff);
