
#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include "Screen.h"
#include "IFF.h"
#include "Gels.h"

int main()
{
    struct Screen *s;

    if (s = otworzEkran())
    {
        struct BitMap *gfx;

        if (gfx = AllocBitMap(320, 256, 5, BMF_INTERLEAVED, NULL))
        {
            PLANEPTR mask;
            if (mask = AllocRaster(320, 256))
            {
                struct IFFHandle *iff;
                if (iff = otworzIFF("Beach/Gfx.iff"))
                {
                    if (przeskanujILBM(iff))
                    {
                        if (wczytajKolory(iff, s->ViewPort.ColorMap))
                        {
                            MakeScreen(s);
                            RethinkDisplay();
                            if (wczytajObrazek(iff, gfx, mask))
                            {
                                struct screenInfo *si = (struct screenInfo *)s->UserData;
                                s->RastPort.BitMap = si->bm[1];
                                rysujIkone(gfx, 0, 0, &s->RastPort, 0, 0, 320, 16);
                                rysujBoba(gfx, 0, 16, gfx, 0, 240, &s->RastPort, 0, 16, 320, 16, mask);

                                WORD x, y;

                                for (y = 2; y < 16; y++)
                                {
                                    for (x = 0; x < 20; x++)
                                    {
                                        if (x == 0 || x == 19 || y == 2 || y == 15)
                                            rysujIkone(gfx, 5 << 4, 16, &s->RastPort, x << 4, y << 4, 16, 16);
                                        else
                                            rysujIkone(gfx, 6 << 4, 16, &s->RastPort, x << 4, y << 4, 16, 16);
                                    }
                                }

                                WaitBlit();
                                while (!ChangeScreenBuffer(s, si->sb[1]))
                                {
                                    WaitTOF();
                                }
                                Delay(400);
                            }
                        }
                    }
                    zamknijIFF(iff);
                }
                FreeRaster(mask, 320, 256);
            }
            FreeBitMap(gfx);
        }
        zamknijEkran(s);
    }
    return(RETURN_OK);
}
