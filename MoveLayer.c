
#define NEWCLIPRECTS_1_1

#include <graphics/layers.h>
#include <graphics/clip.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

void printRect(struct Rectangle *r);
void displayRegion(struct Region *reg);

/* Free all ClipRects in a Layer */

void freeClipRects(struct Layer *l)
{
    struct ClipRect *cr, *nextcr;

    for (cr = l->ClipRect; cr != NULL; cr = nextcr)
    {
        nextcr = cr->Next;

        FreeMem(cr, sizeof(*cr));
    }
}

/* Move layer */

void moveLayer(struct Layer *l, WORD dx, WORD dy)
{
    /* Rebuild ClipRect structure from top layer */
    struct Layer *top;
    struct Region *reg;

    struct Rectangle old = l->bounds;

    l->bounds.MinX += dx;
    l->bounds.MinY += dy;
    l->bounds.MaxX += dx;
    l->bounds.MaxY += dy;

    if (reg = NewRegion())
    {
        for (top = l; top != NULL; top = top->back)
        {
            /* Build obscured ClipRects */
            struct Layer *lobs;
            struct ClipRect *newcr, *prevcr = NULL;
            struct RegionRectangle *rr;

            freeClipRects(top);
            ClearRegion(top->DamageList);

            /* Add full layer */
            OrRectRegion(reg, &top->bounds);

            /* Add obscured areas */
            for (lobs = l->LayerInfo->top_layer; lobs != top; lobs = lobs->back)
            {
                struct Region *aux;
                struct Rectangle rel = { 0 };

                if (aux = NewRegion())
                {
                    OrRectRegion(aux, &lobs->bounds);
                    AndRegionRegion(reg, aux);
                    XorRegionRegion(aux, reg);

                    for (rr = aux->RegionRectangle; rr != NULL; rr = rr->Next)
                    {
                        rel.MinX = rel.MaxX = aux->bounds.MinX;
                        rel.MinY = rel.MaxY = aux->bounds.MinY;

                        rel.MinX += rr->bounds.MinX;
                        rel.MinY += rr->bounds.MinY;
                        rel.MaxX += rr->bounds.MaxX;
                        rel.MaxY += rr->bounds.MaxY;

                        if (newcr = AllocMem(sizeof(*newcr), MEMF_PUBLIC|MEMF_CLEAR))
                        {
                            newcr->bounds = rr->bounds;
                            newcr->Next = prevcr;
                            newcr->lobs = lobs;
                            prevcr = newcr;
                        }
                    }
                    DisposeRegion(aux);
                }
            }

            /* Add final ClipRects */
            for (rr = reg->RegionRectangle; rr != NULL; rr = rr->Next)
            {
                struct Region *aux;
                struct Rectangle rel = { 0 };

                rel.MinX = rel.MaxX = reg->bounds.MinX;
                rel.MinY = rel.MaxY = reg->bounds.MinY;

                rel.MinX += rr->bounds.MinX;
                rel.MinY += rr->bounds.MinY;
                rel.MaxX += rr->bounds.MaxX;
                rel.MaxY += rr->bounds.MaxY;

                if (aux = NewRegion())
                {
                    OrRectRegion(aux, &old);

                    ClearRectRegion(aux, &l->bounds);

                    AndRectRegion(aux, &rel);

                    if (aux->RegionRectangle)
                    {
                        OrRegionRegion(aux, top->DamageList);
                        top->Flags |= LAYERREFRESH;
                    }
                    DisposeRegion(aux);
                }

                if (newcr = AllocMem(sizeof(*newcr), MEMF_PUBLIC|MEMF_CLEAR))
                {
                    newcr->bounds = rel;
                    newcr->Next = prevcr;
                    prevcr = newcr;
                }
            }

            /* Attach */
            top->ClipRect = prevcr;
        }
        DisposeRegion(reg);
    }
}
