
/* Window.c: Auxilliary window functions */

#include <intuition/intuition.h>

#include <clib/intuition_protos.h>
#include <clib/layers_protos.h>

#include "Window.h"

struct Window *openBDWindow(struct Screen *s)
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
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,           IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_GADGETUP,
        TAG_DONE))
    {
        return(w);
    }
    return(NULL);
}

/* moveWindow: This proc moves intuition window by dx/dy */
void moveWindow(struct Window *w, WORD dx, WORD dy)
{
    struct Layer *oldl = w->WLayer; /* Get old layer */
    struct Layer *newl; /* New layer */
    struct Layer_Info *li = &w->WScreen->LayerInfo;
    struct BitMap *bm = w->RPort->BitMap;
    struct Screen *s = w->WScreen;

    struct Rectangle *rect = &oldl->bounds;
    ULONG lock;

    if ((rect->MinX + dx < 0) || (rect->MaxX + dx >= s->Width) || (rect->MinY + dy < 0) || (rect->MaxY + dy >= s->Height))
        return;

    lock = LockIBase(0);

    /* Create layer in new position */
    if (newl = CreateBehindHookLayer(li, bm, rect->MinX + dx, rect->MinY + dy, rect->MaxX + dx, rect->MaxY + dy, LAYERSIMPLE, LAYERS_NOBACKFILL, NULL))
    {
        /* Attach new layer */
        w->WLayer = newl;
        w->RPort = newl->rp;

        /* Attach window */
        newl->Window = w;

        /* Change position */
        w->LeftEdge += dx;
        w->TopEdge += dy;

        MoveLayerInFrontOf(newl, oldl->back);

        /* Delete old layer */
        DeleteLayer(0, oldl);
    }

    UnlockIBase(lock);
}
