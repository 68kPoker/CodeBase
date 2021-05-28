
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include "IFF.h"

struct Window *openScreen()
{
    struct Screen *s;
    struct Rectangle dclip = { 0, 0, 319, 255 };

    if (s = OpenScreenTags(NULL,
        SA_DClip, &dclip,
        SA_Depth, 5,
        SA_DisplayID, LORES_KEY,
        SA_Quiet, TRUE,
        SA_Exclusive, TRUE,
        SA_ShowTitle, FALSE,
        SA_BackFill, LAYERS_NOBACKFILL,
        SA_Interleaved, TRUE,
        TAG_DONE))
    {
        struct Window *w;
        if (w = OpenWindowTags(NULL,
            WA_CustomScreen, s,
            WA_Left, 0,
            WA_Top, 0,
            WA_Width, s->Width,
            WA_Height, s->Height,
            WA_Backdrop, TRUE,
            WA_Borderless, TRUE,
            WA_Activate, TRUE,
            WA_RMBTrap, TRUE,
            WA_SimpleRefresh, TRUE,
            WA_BackFill, LAYERS_NOBACKFILL,
            WA_IDCMP, IDCMP_RAWKEY,
            TAG_DONE))
        {
            return(w);
        }
        CloseScreen(s);
    }
    return(NULL);
}

struct BitMap *paintScreen(struct Window *w, PLANEPTR *mask)
{
    struct BitMap *bm;

    if (bm = loadBitMap("Data/Graphics.iff", w->WScreen->ViewPort.ColorMap, mask))
    {
        MakeScreen(w->WScreen);
        RethinkDisplay();
        blitTile(bm, 0, 0, w->RPort->BitMap, 0, 0, 320, 256);
        return(bm);
    }
    return(NULL);
}

int main()
{
    struct Window *w;
    struct Screen *s;
    PLANEPTR mask;
    struct BitMap *gfx;

    if (w = openScreen())
    {
        if (gfx = paintScreen(w, &mask))
        {
            WaitPort(w->UserPort);
            FreeBitMap(gfx);
            FreeRaster(mask, 320, 256);
        }
        s = w->WScreen;
        CloseWindow(w);
        CloseScreen(s);
    }
    return(0);
}
