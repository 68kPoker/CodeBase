
#include <stdio.h>

#include <intuition/screens.h>
#include <exec/memory.h>
#include <devices/timer.h>

#include <clib/timer_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "Chunky.h"

#define WIDTH 320
#define HEIGHT 256
#define BUFSIZE (WIDTH * HEIGHT)

#define RGB(i) ((i)|((i)<<8)|((i)<<16)|((i)<<24))

int main(void)
{
    UBYTE *mem;
    struct Screen *s;
    struct BitMap aux;
    WORD i;

    if (mem = AllocMem(BUFSIZE, MEMF_CHIP|MEMF_CLEAR))
    {
        InitBitMap(&aux, 8, WIDTH, HEIGHT);
        UBYTE *plane = mem;
        for (i = 0; i < 8; i++)
        {
            aux.Planes[i] = plane;
            plane += (WIDTH >> 3) * HEIGHT;
        }

        if (s = OpenScreenTags(NULL,
            SA_DisplayID, LORES_KEY,
            SA_Width, 320,
            SA_Height, 256,
            SA_Depth, 8,
            SA_BitMap, &aux,
            SA_Quiet,   TRUE,
            SA_ShowTitle, FALSE,
            TAG_DONE))
        {
            UBYTE *buf;

            for (i = 1; i < 256; i++)
            {
                SetRGB32CM(s->ViewPort.ColorMap, i, RGB(i), 0, RGB(i));
            }
            MakeScreen(s);
            RethinkDisplay();

            if (buf = AllocMem(BUFSIZE, MEMF_PUBLIC))
            {
                UBYTE *cur = buf, *end = buf + BUFSIZE;
                WORD i, line, bpr = (WIDTH >> 3) * HEIGHT;
                struct BitMap *bm = s->RastPort.BitMap;
                PLANEPTR plane = bm->Planes[0], plane2 = bm->Planes[4];

                struct EClockVal ecv1, ecv2;
                i = 0;
                line = 0;
                while (cur < end)
                {
                    *cur++ = line;
                    if (++i == 320)
                    {
                        i = 0;
                        line++;
                    }
                }

                ReadEClock(&ecv1);

                WritePixelLine(buf, plane, bpr, bpr, 0);
                WritePixelLine(buf, plane2, bpr, bpr, 1);

                LONG clock = ReadEClock(&ecv2);

                printf("Ticks: %ld (%ld)\n", ecv2.ev_lo - ecv1.ev_lo, clock);
                Delay(100);

                FreeMem(buf, BUFSIZE);
            }
            CloseScreen(s);
        }
        FreeMem(mem, BUFSIZE);
    }
    return 0;
}
