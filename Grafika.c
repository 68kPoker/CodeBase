
/* $Id: Grafika.c,v 1.1 12/.0/.1 .0:.3:.2 Robert Exp Locker: Robert $ */

/* Funkcje graficzne */

#include <graphics/rastport.h>
#include <utility/hooks.h>
#include <hardware/custom.h>
#include <hardware/blit.h>

#include <clib/graphics_protos.h>

#include "Grafika.h"

__far extern struct Custom custom;

extern ULONG hookEntry();

/* Wszystkie funkcje do uûycia przez DoHookClipRects() */

/* 1. Rysowanie kafli */

/* Blitter D=A */

__saveds void drawTileHook(struct Hook *hook, struct RastPort *rp, struct drawMessage *dm)
{
    struct drawInfo *di = (struct drawInfo *)hook->h_Data;
    struct Custom *c = &custom;
    WORD srcbpr = di->src->BytesPerRow; /* Offset to next line */
    WORD destbpr = rp->BitMap->BytesPerRow; /* Offset to next line */
    WORD destx = dm->rect.MinX;
    WORD desty = dm->rect.MinY;
    WORD srcx  = di->srcx + dm->ox - di->destx;
    WORD srcy  = di->srcy + dm->oy - di->desty;

    WORD width = dm->rect.MaxX - dm->rect.MinX + 1;
    WORD height = dm->rect.MaxY - dm->rect.MinY + 1;

    WORD i, depth = rp->BitMap->Depth;
    LONG srcoffset = (srcbpr * srcy) + (srcx >> 3);
    LONG destoffset = (destbpr * desty) + (destx >> 3);

    PLANEPTR *srcplanes = di->src->Planes;
    PLANEPTR *destplanes = rp->BitMap->Planes;

    OwnBlitter();

    for (i = 0; i < depth; i++)
    {
        WaitBlit();
        c->bltcon0 = SRCA | DEST | 0xf0;
        c->bltcon1 = 0;
        c->bltapt  = srcplanes[i] + srcoffset;
        c->bltdpt  = destplanes[i] + destoffset;
        c->bltamod = srcbpr - (width >> 3);
        c->bltdmod = destbpr - (width >> 3);
        c->bltafwm = 0xffff;
        c->bltalwm = 0xffff;
        c->bltsizv = height;
        c->bltsizh = width >> 4;
    }

    DisownBlitter();
}

void bltTileRastPort(struct BitMap *srcbm, WORD srcx, WORD srcy, struct RastPort *destrp, WORD destx, WORD desty, WORD width, WORD height)
{
    struct Hook hook;
    struct drawInfo di;
    struct Rectangle rect = { destx, desty, destx + width - 1, desty + height - 1 };

    di.src = srcbm;
    di.srcx = srcx;
    di.srcy = srcy;
    di.destx = destx;
    di.desty = desty;

    hook.h_Data = (APTR)&di;
    hook.h_Entry = hookEntry;
    hook.h_SubEntry = (HOOKFUNC)drawTileHook;

    DoHookClipRects(&hook, destrp, &rect);
}
