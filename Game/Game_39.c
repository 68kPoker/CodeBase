
#include <stdio.h>
#include <exec/types.h>
#include <graphics/gfx.h>
#include <hardware/blit.h>
#include <hardware/custom.h>

#include <intuition/screens.h>
#include <intuition/intuition.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "Blitter.h"

__far extern struct Custom custom;

/* Draw line into BitMap */
void drawLine(struct BitMap *bm, WORD x1, WORD y1, WORD x2, WORD y2, UBYTE mask, UBYTE minterm)
{
    WORD dx = x2 - x1, dy = y2 - y1;
    WORD oct[3];
    WORD bpr = bm->BytesPerRow;
    LONG offset = PlaneOffset(x1, y1, bpr);
    UBYTE i, depth = bm->Depth;
    PLANEPTR plane;

    Octant(dx, dy, oct);

    OwnBlitter();

    for (i = 0; i < depth; i++)
    {
        if (!(mask & (1 << i)))
        {
            continue;
        }

        plane = bm->Planes[i] + offset;

        WaitBlit();
        LineMode(oct, plane, x1 & 15, minterm, bpr);
    }

    DisownBlitter();
}

void loop(struct Window *w)
{
    struct MsgPort *mp = w->UserPort;
    WaitPort(mp);


}

int main(void)
{
    struct Screen *s;
    UWORD pens[] = { ~0 };

    if (s = OpenScreenTags(NULL,
        SA_Pens,        pens,
        SA_Depth,       DEPTH,
        SA_Title,       "Game Screen",
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        struct BitMap *bm = s->RastPort.BitMap;
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
            WA_SimpleRefresh,   TRUE,
            WA_BackFill,        LAYERS_NOBACKFILL,
            WA_IDCMP,           IDCMP_MOUSEBUTTONS,
            TAG_DONE))
        {
            drawLine(bm, 0, 0, s->Width - 1, 0, 0x1, NORMAL);
            drawLine(bm, s->Width - 1, 1, s->Width - 1, s->Height - 1, 0x1, NORMAL);
            drawLine(bm, s->Width - 2, s->Height - 1, 0, s->Height - 1, 0x1, NORMAL);
            drawLine(bm, 0, s->Height - 2, 0, 1, 0x1, NORMAL);

            loop(w);

            CloseWindow(w);
        }
        CloseScreen(s);
    }

    return 0;
}
