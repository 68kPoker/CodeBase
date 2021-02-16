
/* Game.c: Main game code */

#include <exec/interrupts.h>
#include <libraries/iffparse.h>

#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "Screen.h"
#include "IFF.h"
#include "Window.h"
#include "Tile.h"

int main(void)
{
    struct BitMap *bm[2];
    struct Interrupt is;
    struct copperData cd;

    if (bm[0] = allocBitMap())
    {
        if (bm[1] = allocBitMap())
        {
            struct Screen *s;

            if (s = openScreen(bm[0]))
            {
                struct IFFHandle *iff;
                if (iff = openIFF("Data/Graphics.iff", IFFF_READ))
                {
                    if (scanILBM(iff))
                    {
                        if (loadCMAP(iff, s))
                        {
                            struct BitMap *gfx;
                            if (gfx = loadBitMap(iff))
                            {
                                struct Window *w;
                                if (w = openBDWindow(s))
                                {
                                    if (addCopperList(&s->ViewPort))
                                    {
                                        if (addCopperInt(&is, &cd, &s->ViewPort))
                                        {
                                            WORD i, j;
                                            for (i = 0; i < 5; i++)
                                                BltBitMap(gfx, 0, 0, bm[1], i << 6, 0, 64, 16, 0xc0, 0xff, NULL);

                                            for (j = 1; j < 16; j++)
                                                for (i = 0; i < 20; i++)
                                                    if (i == 0 || i == 19 || j == 1 || j == 15)
                                                        BltBitMap(gfx, 16, 128, bm[1], i << 4, j << 4, 16, 16, 0xc0, 0xff, NULL);
                                                    else
                                                        BltBitMap(gfx, 0, 128, bm[1], i << 4, j << 4, 16, 16, 0xc0, 0xff, NULL);

                                            SetSignal(0L, 1L << cd.signal);
                                            Wait(1L << cd.signal);
                                            drawTile(bm[1], 0, 0, bm[0], 0, 0, 320, 256);
                                            Delay(300);
                                            remCopperInt(&is);
                                        }
                                    }
                                    CloseWindow(w);
                                }
                                FreeBitMap(gfx);
                            }
                        }
                    }
                    closeIFF(iff);
                }
                CloseScreen(s);
            }
            FreeBitMap(bm[1]);
        }
        FreeBitMap(bm[0]);
    }
    return(0);
}
