
/* $Log:	Gels.c,v $
 * Revision 1.1  12/.1/.0  .1:.0:.3  Robert
 * Initial revision
 *  */

/* Elementy graficzne to ikony oraz ikony dwuwarstwowe (BOBy) */

#include <hardware/custom.h>
#include <hardware/blit.h>

#include <clib/graphics_protos.h>

#include "Gels.h"

__far extern struct Custom custom;

/* Podstawowa funkcja rysujâca ikonë */

void rysujIkone(struct BitMap *bm, WORD srcx, WORD srcy,
    struct RastPort *rp, WORD destx, WORD desty, WORD width, WORD height)
{
    struct Custom *cust = &custom;

    OwnBlitter();

    /* Kafel */
    srcx  = (srcx  + 15) >> 4;
    destx = (destx + 15) >> 4;
    width = (width + 15) >> 4;

    WaitBlit();

    cust->bltcon0 = A_TO_D | SRCA | DEST;
    cust->bltcon1 = 0;
    cust->bltapt  = bm->Planes[0] + (srcy * bm->BytesPerRow) + (srcx << 1);
    cust->bltdpt  = rp->BitMap->Planes[0] + (desty * rp->BitMap->BytesPerRow) + (destx << 1);
    cust->bltamod = (bm->BytesPerRow / bm->Depth) - (width << 1);
    cust->bltdmod = (rp->BitMap->BytesPerRow / rp->BitMap->Depth) - (width << 1);
    cust->bltafwm = 0xffff;
    cust->bltalwm = 0xffff;
    cust->bltsizv = height * bm->Depth;
    cust->bltsizh = width;

    DisownBlitter();
}

/* Funkcja rysujâca ikonë dwuwarstwowâ */

void rysujBoba(struct BitMap *bm, WORD srcx, WORD srcy, struct BitMap *back,
    WORD backx, WORD backy, struct RastPort *rp, WORD destx, WORD desty,
    WORD width, WORD height, PLANEPTR mask)
{
    struct Custom *cust = &custom;
    WORD plane;
    UWORD lastmask = 0xffff;
    WORD modulo = 0, shift;

    OwnBlitter();

    srcx  = (srcx  + 15) >> 4; /* Kafel */
    backx = (backx + 15) >> 4;
    width = (width + 15) >> 4;

    if (shift = (destx & 0xf))
    {
        lastmask = 0x0000;
        width++;
        modulo -= 2;
    }

    destx = (destx + 15) >> 4;

    for (plane = 0; plane < bm->Depth; plane++)
    {
        WaitBlit();

        cust->bltcon0 = (0xca | SRCA | SRCB | SRCC | DEST) | (shift << ASHIFTSHIFT);
        cust->bltcon1 = shift << BSHIFTSHIFT;
        cust->bltapt  = mask + (srcy * bm->BytesPerRow) + (srcx << 1);
        cust->bltbpt  = bm->Planes[plane] + (srcy * bm->BytesPerRow) + (srcx << 1);
        cust->bltcpt  = back->Planes[plane] + (backy * back->BytesPerRow) + (backx << 1);
        cust->bltdpt  = rp->BitMap->Planes[plane] + (desty * rp->BitMap->BytesPerRow) + (destx << 1);
        cust->bltamod = bm->BytesPerRow - (width << 1) + modulo;
        cust->bltbmod = bm->BytesPerRow - (width << 1) + modulo;
        cust->bltcmod = back->BytesPerRow - (width << 1) + modulo;
        cust->bltdmod = rp->BitMap->BytesPerRow - (width << 1) + modulo;
        cust->bltafwm = 0xffff;
        cust->bltalwm = lastmask;
        cust->bltsize = (height << 6) | width;
    }

    DisownBlitter();
}
