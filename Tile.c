
/* Tile.c: Tile drawing functions */

#include <hardware/custom.h>
#include <graphics/gfx.h>

#include <clib/graphics_protos.h>

#include "Tile.h"

__far extern struct Custom custom;

/* drawTile: Draw 16-pixel width aligned interleaved bitmap */
void drawTile(struct BitMap *gfx, UWORD sx, UWORD sy, struct BitMap *bm, UWORD dx, UWORD dy, UWORD width, UWORD height)
{
    LONG srcoffset = (gfx->BytesPerRow * sy) + ((sx >> 4) << 1);
    LONG dstoffset = (bm->BytesPerRow * dy) + ((dx >> 4) << 1);
    WORD wordWidth = (width + 15) >> 4;
    WORD srcmodulo = gfx->Planes[1] - gfx->Planes[0] - (wordWidth << 1);
    WORD dstmodulo = bm->Planes[1] - bm->Planes[0] - (wordWidth << 1);

    OwnBlitter();

    WaitBlit();

    custom.bltcon0 = SRCA | DEST | A_TO_D;
    custom.bltcon1 = 0;
    custom.bltapt  = gfx->Planes[0] + srcoffset;
    custom.bltdpt  = bm->Planes[0] + dstoffset;
    custom.bltamod = srcmodulo;
    custom.bltdmod = dstmodulo;
    custom.bltafwm = 0xffff;
    custom.bltalwm = 0xffff;
    custom.bltsizv = height * gfx->Depth;
    custom.bltsizh = wordWidth;

    DisownBlitter();
}
