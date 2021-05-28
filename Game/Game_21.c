
#include "Game.h"

#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

VOID drawScreen(struct GameInfo *gi)
{
    struct BitMap *gfx = gi->pi.bitmap, *dest = gi->si.bitmaps[0];

    BltBitMap(gfx, 0, 0, dest, 0, 0, 320, 256, 0xc0, 0xff, NULL);
}

LONG setupScreen(struct GameInfo *gi, UWORD width, UWORD height, UBYTE depth)
{
    if (loadPicture("Data/Bar.iff", &gi->pi) == 0)
        {
        if (allocScreenBitMaps(&gi->si, width, height, depth) == 0)
            {
            /* Draw initial contents */
            drawScreen(gi);

            /* Open screen with contents and colors */
            if (openScreen(&gi->si, &gi->pi) == 0)
                {
                return(0);
                }
            freeScreenBitMaps(&gi->si);
            }
        freePicture(&gi->pi);
        }
    return(-1);
}

LONG cleanupScreen(struct GameInfo *gi)
{
    closeScreen(&gi->si);
    freeScreenBitMaps(&gi->si);
    freePicture(&gi->pi);
}

int main(void)
{
    struct GameInfo game;

    if (setupScreen(&game, 320, 256, 5) == 0)
        {
        Delay(500);
        cleanupScreen(&game);
        }
    return(0);
}
