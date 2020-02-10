
/*
**  (C)2018-2020 Robert Szacki Software House
**
**  » Magazyn «
**
**  $Id$
*/

#include <hardware/custom.h>
#include <graphics/gfx.h>
#include <clib/graphics_protos.h>

#include "Windows.h"
#include "Screen.h"

extern __far struct Custom custom;

void blitBitMap(struct BitMap *bm, WORD srcx, WORD srcy, struct windowInfo *wi, WORD destx, WORD desty, WORD width, WORD height)
{
    struct Custom *cust = &custom;
    WORD depth = bm->Depth, plane;
    struct BitMap *destbm = wi->si->bm[wi->si->frame];

    if (destx + width > wi->width)
    {
        width = wi->width - destx;
    }
    if (desty + height > wi->height)
    {
        height = wi->height - desty;
    }

    OwnBlitter();

    for (plane = 0; plane < depth; plane++)
    {
        WaitBlit();
        cust->bltcon0 = 0x09f0;
        cust->bltcon1 = 0x0000;
        cust->bltapt  = bm->Planes[plane] + (srcy * bm->BytesPerRow) + (srcx >> 3);
        cust->bltdpt  = destbm->Planes[plane] + ((desty + wi->top) * destbm->BytesPerRow) + ((destx + wi->left) >> 3);
        cust->bltamod = bm->BytesPerRow - (width >> 3);
        cust->bltdmod = destbm->BytesPerRow - (width >> 3);
        cust->bltafwm = 0xffff;
        cust->bltalwm = 0xffff;
        cust->bltsizv = height;
        cust->bltsizh = width >> 4;
    }
    DisownBlitter();
}

void drawBackground(struct windowInfo *wi, struct BitMap *bm, WORD srcleft, WORD srctop, WORD width, WORD height)
{
    WORD left, top;

    for (top = 0; top < wi->height; top += height)
    {
        for (left = 0; left < wi->width; left += width)
        {
            blitBitMap(bm, srcleft, srctop, wi, left, top, width, height);
        }
    }
}
