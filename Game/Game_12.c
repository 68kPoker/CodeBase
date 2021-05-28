
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include "Config.h"
#include "Video.h"
#include "Data.h"

int main()
{
    struct config c;
    struct screen s;
    struct window w;
    struct graphics gfx;
    struct ReadArgs *rda;

    if (getConfig(&c, NULL))
    {
        ULONG array[5] = { 0 };
        STRPTR template = "PUB/S,PUBNAME/K,WIDTH/K/N,HEIGHT/K/N,DEPTH/K/N";
        if (rda = ReadArgs(template, array, NULL))
        {
            c.pub = array[0];
            c.pubName = (STRPTR)array[1];
            if (array[2])
                c.width = *(ULONG *)array[2];
            if (array[3])
                c.height = *(ULONG *)array[3];
            if (array[4])
                c.depth = *(ULONG *)array[4];

            printf("Width = %d\nHeight = %d\nDepth = %d\n", c.width, c.height, c.depth);
        }
        else
        {
            printf("Arguments unsuitable for the template: %s\n", template);
            return(RETURN_WARN);
        }
        if (openScreen(&s, &c))
        {
            if (openWindow(&w, &s, &c))
            {
                SetWindowTitles(w.window, (STRPTR)~0, "Loading graphics...");
                if (loadGraphics(&gfx, "Data/Magazyn.pic", &w))
                {
                    SetWindowTitles(w.window, (STRPTR)~0, "Remapping graphics...");
                    processWindow(&w, &gfx);

                    SetWindowTitles(w.window, (STRPTR)~0, "Welcome to Warehouse!");
                    WORD i;

                    if (gfx.bitmap)
                    {
                        for (i = 0; i < c.height; i++)
                        {
                            BltBitMapRastPort(gfx.bitmap, 0, i, w.window->RPort, 0, i, w.window->Width, 1, 0xc0);
                            WaitTOF();
                        }
                    }
                    WaitPort(w.window->UserPort);
                    unloadGraphics(&gfx);
                }
                closeWindow(&w);
            }
            closeScreen(&s);
        }
        if (rda)
        {
            FreeArgs(rda);
        }
    }
    return(RETURN_OK);
}
