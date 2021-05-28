
#include <exec/memory.h>
#include <graphics/gfx.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>

#include "Win.h"

void initBounds(struct Rectangle *bounds, UWORD x, UWORD y, UWORD width, UWORD height)
{
    bounds->MinX = x;
    bounds->MinY = y;
    bounds->MaxX = x + width - 1;
    bounds->MaxY = y + height - 1;
}

BOOL initWindow(struct window *w, UWORD x, UWORD y, UWORD width, UWORD height, void (*draw)(struct window *w, struct RastPort *rp, UWORD update), struct RastPort *rp)
{
    initBounds(&w->bounds[0], x, y, width, height);
    initBounds(&w->bounds[1], x, y, width, height);

    w->draw = draw;
    w->update = REDRAW | SYNC;

    return(TRUE);
}

void initScreen(struct screen *s, UWORD width, UWORD height, void (*draw)(struct window *w, struct RastPort *rp, UWORD update), struct RastPort *rp)
{
    initBounds(&s->bounds, 0, 0, width, height);
    initWindow(&s->back, 0, 0, width, height, draw, rp);
    s->front = &s->back;
    s->back.front = NULL;
    s->back.back = NULL;

    drawWindow(&s->back, rp, s->back.update, NULL);

    s->back.update &= ~REDRAW;
}

struct window *openWindow(struct screen *s, UWORD x, UWORD y, UWORD width, UWORD height, void (*draw)(struct window *w, struct RastPort *rp, UWORD update), struct RastPort *rp)
{
    struct window *w;

    if (w = AllocMem(sizeof(*w), MEMF_PUBLIC|MEMF_CLEAR))
    {
        initWindow(w, x, y, width, height, draw, rp);
        w->back = s->front;
        s->front->front = w;
        w->front = NULL;
        s->front = w;

        drawWindow(w, rp, w->update, NULL);

        w->update &= ~REDRAW;

        return(w);
    }
    return(NULL);
}

BOOL drawWindow(struct window *w, struct RastPort *rp, UWORD update, struct Region *aux)
{
    /* Clip */
    struct Region *reg, *prev;
    struct window *v;

    if (reg = NewRegion())
    {
        /* Add this window */
        OrRectRegion(reg, &w->bounds);

        /* Subtract front */
        for (v = w->front; v != NULL; v = v->front)
        {
            ClearRectRegion(reg, &v->bounds);
        }

        if (aux)
        {
            AndRegionRegion(aux, reg);
        }

        prev = InstallClipRegion(rp->Layer, reg);
        w->draw(w, rp, update);
        InstallClipRegion(rp->Layer, prev);

        DisposeRegion(reg);
        return(TRUE);
    }
    return(FALSE);
}

BOOL drawWindows(struct screen *s, struct RastPort *rp, struct window *clip, struct Region *aux)
{
    struct window *w;

    for (w = &s->back; w != NULL; w = w->front)
    {
        if (w->update & (UPDATE | SYNC))
        {
            if (w->update & UPDATE)
                w->update |= SYNC; /* Change sync status */
            else
                w->update &= ~SYNC;

            w->update &= ~UPDATE; /* No need of further update unless changed */

            if (w != clip)
            {
                drawWindow(w, rp, w->update, aux);
            }
            else
            {
                drawWindow(w, rp, w->update, NULL);
            }

            w->update &= ~REDRAW;
        }
    }
    return(TRUE);
}

void moveWindow(struct screen *s, struct window *w, WORD dx, WORD dy, struct RastPort *rp)
{
    struct window *v;
    struct Rectangle bounds = w->bounds;
    struct Region *aux;

    if (aux = NewRegion())
    {
        OrRectRegion(aux, &bounds);

        bounds.MinX += dx;
        bounds.MinY += dy;
        bounds.MaxX += dx;
        bounds.MaxY += dy;

        w->bounds = bounds;

        ClearRectRegion(aux, &bounds);

        for (v = w; v != NULL; v = v->back)
            v->update |= (UPDATE | REDRAW);

        drawWindows(s, rp, w, aux);

        DisposeRegion(aux);
    }
}

void closeWindow(struct screen *s, struct window *w, struct RastPort *rp)
{
    struct Region *aux;

    if (aux = NewRegion())
    {
        if (w == &s->back)
            return;

        if (w->front)
            w->front->back = w->back;
        else
            s->front = w->back;

        if (w->back)
            w->back->front = w->front;

        if (rp)
            drawWindows(s, rp, w, aux);

        FreeMem(w, sizeof(*w));
        DisposeRegion(aux);
    }
}

void closeWindows(struct screen *s)
{
    while (s->front != &s->back)
    {
        closeWindow(s, s->front, NULL);
    }
}
