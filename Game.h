
#include <exec/types.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

struct tileInfo
{
    UBYTE floorGfx, objectGfx;
    BOOL updateFlag;
};

struct boardInfo
{
    struct tileInfo tileArray[BOARD_HEIGHT][BOARD_WIDTH];
};
