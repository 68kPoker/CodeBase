
#include "screen.h"
#include "windows.h"
#include "iff.h"

#include <graphics/gfxmacros.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>

void displayNumber(struct RastPort *rp, WORD x, WORD y, WORD num)
{
    UBYTE text = '0' + num;

    SetABPenDrMd(rp, 2, 18, JAM2);
    Move(rp, x, y);
    Text(rp, &text, 1);
}

void test(struct windowInfo *wi)
{
    struct RastPort *rp = wi->w->RPort;
/*
    struct RastPort *rp = &si->s->RastPort;
    WORD i;
    UBYTE text[5];

    for (i = 0; i < 100; i++)
        {
        rp->BitMap = si->sb[si->dbuf.frame]->sb_BitMap;

        safeDBuf(&si->dbuf, SAFE);

        Move(rp, 0, si->font->tf_Baseline);
        SetAPen(rp, 1);
        sprintf(text, "%4d", i);
        Text(rp, text, 4);

        changeScreen(si);
        }
*/

    WORD x, y, z;

    BltBitMapRastPort(wi->gfx, 0, 0, rp, 0, 0, 320, 16, 0xc0);

    BltBitMapRastPort(wi->gfx, 3 << 4, 16, rp, 16, 0, 16, 16, 0xc0);

    displayNumber(rp, 40, 4 + rp->Font->tf_Baseline, wi->trigger);

    for (x = 0; x < 20; x++)
        {
        for (y = 1; y < 16; y++)
            {
            WORD tile;
            tile = TILE_FLOOR;
            if (x == 0 || x == 19 || y == 1 || y == 15)
               {
               tile = TILE_WALL;
               }
            BltBitMapRastPort(wi->gfx, tile << 4, 16, rp, 0, y << 4, 16, 16, 0xc0);
            }
        ScrollRasterBF(rp, -16, 0, 0, 16, 319, 255);
        WaitTOF();
        }

    handleWindow(wi);
}

int main(void)
{
    struct screenInfo si = { 0 };
    struct windowInfo wi = { 0 };
    struct ILBMInfo ii = { 0 };

    if (loadILBM(&ii, "Data/Bar.iff"))
        {
        if (openScreen(&si, &ii))
            {
            if (openWindow(&wi, si.s, WID_BACKDROP))
                {
                wi.gfx = ii.bm;
                test(&wi);
                closeWindow(&wi);
                }
            closeScreen(&si);
            }
        freeILBM(&ii);
        }
    return(0);
}
