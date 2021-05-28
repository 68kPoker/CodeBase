
#include <intuition/intuition.h>
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "Screen.h"
#include "Windows.h"

BOOL openWindow(struct windowInfo *wi, struct screenInfo *si, WORD left, WORD top, WORD width, WORD height, ULONG idcmp)
{
    UBYTE depth = GetBitMapAttr(si->s->RastPort.BitMap, BMA_DEPTH);

    if (wi->back = AllocBitMap(width, height, depth, 0, NULL))
    {
        wi->cx = 0;
        wi->cy = 0;
        BltBitMap(si->s->RastPort.BitMap, left, top, wi->back, 0, 0, width, height, 0xc0, 0xff, NULL);
        if (wi->w = OpenWindowTags(NULL,
            WA_CustomScreen,    si->s,
            WA_Left,            left,
            WA_Top,             top,
            WA_Width,           width,
            WA_Height,          height,
            WA_Borderless,      TRUE,
            WA_Activate,        TRUE,
            WA_RMBTrap,         TRUE,
            WA_SimpleRefresh,   TRUE,
            WA_BackFill,        LAYERS_NOBACKFILL,
            WA_IDCMP,           idcmp,
            TAG_DONE))
        {
            wi->w->UserData = (APTR)wi;
            return (TRUE);
        }
        FreeBitMap(wi->back);
    }
    return (FALSE);
}

void closeWindow(struct windowInfo *wi)
{
    struct Window *w = wi->w;
    struct Screen *s = w->WScreen;

    BltBitMap(wi->back, 0, 0, s->RastPort.BitMap, w->LeftEdge, w->TopEdge, w->Width, w->Height, 0xc0, 0xff, NULL);
    CloseWindow(wi->w);
    FreeBitMap(wi->back);
}

BOOL moveWindow(struct windowInfo *wi, WORD dx, WORD dy)
{
    struct Window *w = wi->w, *nw;
    struct Screen *s = w->WScreen;
    struct BitMap *dest = s->RastPort.BitMap;

    WORD nx = w->LeftEdge + dx, ny = w->TopEdge + dy;

    if (nx < 0)
        nx = 0;
    else if (nx > (s->Width - w->Width))
        nx = s->Width - w->Width;

    if (ny < 0)
        ny = 0;
    else if (ny > (s->Height - w->Height))
        ny = s->Height - w->Height;

    dx = nx - w->LeftEdge;
    dy = ny - w->TopEdge;

    if (dx == 0 && dy == 0)
        return;

    if (nw = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            nx,
        WA_Top,             ny,
        WA_Width,           w->Width,
        WA_Height,          w->Height,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_IDCMP,           w->IDCMPFlags,
        TAG_DONE))
    {
        if (dy > 0)
        {
            BltBitMap(wi->back, 0, 0, dest, w->LeftEdge, w->TopEdge, w->Width, dy, 0xc0, 0xff, NULL);
        }
        else if (dy < 0)
        {
            BltBitMap(wi->back, 0, w->Height + dy, dest, w->LeftEdge, w->TopEdge + w->Height + dy, w->Width, -dy, 0xc0, 0xff, NULL);
        }

        if (dx > 0)
        {
            BltBitMap(wi->back, 0, MAX(0, dy), dest, w->LeftEdge, w->TopEdge + MAX(0, dy), dx, w->Height - ABS(dy), 0xc0, 0xff, NULL);
        }
        else if (dx < 0)
        {
            BltBitMap(wi->back, w->Width + dx, MAX(0, dy), dest, w->LeftEdge + w->Width + dx, w->TopEdge + MAX(0, dy), -dx, w->Height - ABS(dy), 0xc0, 0xff, NULL);
        }
        CloseWindow(w);
        wi->w = nw;
        return (TRUE);
    }
    return (FALSE);
}
