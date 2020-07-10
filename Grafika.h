
#include <exec/types.h>
#include <graphics/gfx.h>

/* Moje funkcje graficzne */

struct drawMessage
{
    struct Layer     *layer;
    struct Rectangle rect;
    LONG             ox, oy;
};

struct drawInfo
{
    struct BitMap *src;
    WORD srcx, srcy, destx, desty;
};

void bltTileRastPort(struct BitMap *srcbm, WORD srcx, WORD srcy, struct RastPort *destrp, WORD destx, WORD desty, WORD width, WORD height);
void drawTileHook(struct Hook *hook, struct RastPort *rp, struct drawMessage *dm);
