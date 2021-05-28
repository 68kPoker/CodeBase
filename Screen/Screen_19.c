
#include "IFF.h"

#include <dos/dos.h>
#include <intuition/screens.h>

#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

struct Screen *openScreen(STRPTR pic)
{
    struct ILBMInfo ilbm;

    if (loadILBM(&ilbm, pic))
    {
        struct Screen *s;
        struct BitMap *bm = ilbm.bm;

        if (s = OpenScreenTags(NULL,
            SA_Width,   320,
            SA_Height,  256,
            SA_Depth,   GetBitMapAttr(bm, BMA_DEPTH),
            SA_DisplayID,   LORES_KEY,
            SA_Quiet,   TRUE,
            SA_Exclusive,   TRUE,
            SA_Draggable,   FALSE,
            SA_ShowTitle,   FALSE,
            SA_BackFill,    LAYERS_NOBACKFILL,
            SA_Colors32,    ilbm.colors,
            TAG_DONE))
        {
            BltBitMap(bm, 0, 0, s->RastPort.BitMap, 0, 0, 320, 256, 0xc0, 0xff, NULL);
            WaitBlit();
            unloadILBM(&ilbm);
            return(s);
        }
        unloadILBM(&ilbm);
    }
    return(NULL);
}

int main()
{
    struct Screen *s;

    if (s = openScreen("Dane/Magazyn.iff"))
    {
        Delay(400);
        CloseScreen(s);
    }
    return(RETURN_OK);
}
