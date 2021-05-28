
#include <exec/types.h>
#include <intuition/classusr.h>

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

enum
{
    GID_CLOSE,
    GID_LOAD,
    GID_SAVE,
    GID_HERO,
    GID_BOX,
    GID_WALL,
    GID_FLAGSTONE,
    GID_FLOOR,
    GID_COUNT
};

enum
{
    T_FLOOR,
    T_WALL,
    T_FLAGSTONE,
    T_BOX,
    T_HERO,
    TILES
};

struct screenInfo
{
    struct BitMap *bm[2];
    struct ScreenBuffer *sb[2];
    struct MsgPort *mp;
    BOOL safe;
    WORD frame;
    struct Window *w;
};

struct board
{
    WORD tiles[BOARD_HEIGHT][BOARD_WIDTH];
};

struct boardInfo
{
    struct board *board;
    Point heropos;
    Class *cl;
    Object *images[GID_COUNT];
    Object *gadgets[GID_COUNT];
};

ULONG *getPalette(struct ColorMap *cm);
struct Window *openScreen(struct screenInfo *si, ULONG *pal, void (*draw)(struct BitMap *bm), struct boardInfo *bi);
void safeToDraw(struct Window *w);
void changeBuffer(struct Window *w);
void closeScreen(struct Window *w);
