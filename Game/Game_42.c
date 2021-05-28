
#include <graphics/modeid.h>
#include <clib/graphics_protos.h>

#include "Screen.h"
#include "Windows.h"
#include "Game.h"

void prepGfx(struct BitMap *bitMap)
{
    struct RastPort rPort;

    InitRastPort(&rPort);
    rPort.BitMap = bitMap;

    SetAPen(&rPort, 3);
    RectFill(&rPort, 0, 0, 15, 15);
}

int main(void)
{
    ULONG modeID;
    const WORD rasWidth = 320, rasHeight = 256, depth = 5;
    static struct boardInfo board = { 0 };

    struct Screen *screen;
    struct screenInfo scrInfo;

    if ((modeID = obtainModeID(rasWidth, rasHeight, depth)) != INVALID_ID)
    {
        if (screen = openScreen("Magazyn", modeID, depth, &scrInfo))
        {
            if (addCopper(&scrInfo, 1))
            {
                if (addRegions(&scrInfo))
                {
                    struct Window *win;
                    if (win = openWindow(screen))
                    {
                        if (scrInfo.gfxBitMap = AllocBitMap(320, 256, 5, 0, NULL))
                        {
                            prepGfx(scrInfo.gfxBitMap);
                            mainLoop(win, &board);
                            FreeBitMap(scrInfo.gfxBitMap);
                        }
                        CloseWindow(win);
                    }
                    remRegions(&scrInfo);
                }
                remCopper(&scrInfo);
            }
            closeScreen(screen);
        }
    }
    return(0);
}
