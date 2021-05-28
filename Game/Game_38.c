
#include <stdio.h>

#include <intuition/intuition.h>

#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "Win.h"

#include "debug.h"

void myDraw(struct window *w, struct RastPort *rp, UWORD update)
{
    static UBYTE count = 0;
    if (update & REDRAW)
    {
        SetAPen(rp, count);
        count++;
        SetOutlinePen(rp, 1);

        RectFill(rp, w->bounds.MinX, w->bounds.MinY, w->bounds.MaxX, w->bounds.MaxY);
    }
}

int main(void)
{
    struct Screen *s;
    struct Rectangle dclip = { 0, 0, 319, 255 };

    if (s = OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_Depth,       5,
        SA_DisplayID,   LORES_KEY,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        struct Window *w;

        if (w = OpenWindowTags(NULL,
            WA_CustomScreen,    s,
            WA_Left,            0,
            WA_Top,             0,
            WA_Width,           s->Width,
            WA_Height,          s->Height,
            WA_Backdrop,        TRUE,
            WA_Borderless,      TRUE,
            WA_Activate,        TRUE,
            WA_RMBTrap,         TRUE,
            WA_IDCMP,           IDCMP_RAWKEY,
            WA_SimpleRefresh,   TRUE,
            WA_BackFill,        LAYERS_NOBACKFILL,
            TAG_DONE))
        {
            struct RastPort *rp = w->RPort;
            struct screen screen;
            struct window *win;

            initScreen(&screen, s->Width, s->Height, myDraw, rp);

            if (win = openWindow(&screen, 64, 64, 128, 128, myDraw, rp))
            {
                Delay(200);
                moveWindow(&screen, win, 64, -32, rp);
            }

            WaitPort(w->UserPort);

            closeWindows(&screen);

            CloseWindow(w);
        }

        CloseScreen(s);
    }
    return(0);
}
