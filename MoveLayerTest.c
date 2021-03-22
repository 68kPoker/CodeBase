
#include <stdlib.h>
#include <stdio.h>

#include <intuition/intuition.h>
#include <graphics/rpattr.h>
#include <clib/layers_protos.h>
#include <clib/intuition_protos.h>

BOOL moveLayer(struct Layer *l, WORD dx, WORD dy);
void printRect(struct Rectangle *r);

void printRect(struct Rectangle *r)
{
    printf("[%-3d %-3d %-3d %-3d]\n", r->MinX, r->MinY, r->MaxX, r->MaxY);
}

void displayClipRects(struct ClipRect *cr)
{
    printf("\tClipRects:\n");
    while (cr)
    {
        printf("\t\t[%3d %3d %3d %3d] ", cr->bounds.MinX, cr->bounds.MinY, cr->bounds.MaxX, cr->bounds.MaxY);
        printf("\t\t$%x\n", cr->lobs);
        cr = cr->Next;
    }
    putchar('\n');
}

void displayRegion(struct Region *reg)
{
    struct RegionRectangle *rr = reg->RegionRectangle;

    printRect(&reg->bounds);

    while (rr)
    {
        printRect(&rr->bounds);
        rr = rr->Next;
    }
}

void displayLayer(struct Layer *l)
{
    printf("Layer:\n");
    displayClipRects(l->ClipRect);
}

void displayLayerInfo(struct Layer_Info *li)
{
    struct Layer *l;

    for (l = li->top_layer; l != NULL; l = l->back)
    {
        displayLayer(l);
    }
}

void openWindow2(struct Window *w)
{
    struct Window *v;

    if (v = OpenWindowTags(NULL,
        WA_CustomScreen,    w->WScreen,
        WA_Left,            0,
        WA_Top,             w->WScreen->BarHeight + 1 - 6,
        WA_Width,           320,
        WA_Height,          160,
        WA_Activate,        TRUE,
        WA_Title,           "Simple Window",
        WA_SimpleRefresh,   TRUE,
        WA_CloseGadget,     TRUE,
        WA_RMBTrap,         TRUE,
        WA_IDCMP,           IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS,
        TAG_DONE))
    {
        struct Layer_Info *li = &w->WScreen->LayerInfo;
        struct Rectangle rect;
        struct IntuiMessage *msg;
        ULONG lock;
        struct Layer *l;
        BOOL done = FALSE;

        while (!done)
        {
            WaitPort(v->UserPort);

            while (msg = (struct IntuiMessage *)GetMsg(v->UserPort))
            {
                ULONG class = msg->Class;
                WORD code = msg->Code;
                WORD mx = msg->MouseX;
                WORD my = msg->MouseY;

                ReplyMsg((struct Message *)msg);

                if (class == IDCMP_CLOSEWINDOW)
                {
                    done = TRUE;
                }
                else if (class == IDCMP_MOUSEBUTTONS && code == IECODE_RBUTTON)
                {
                    WORD dx = mx;
                    WORD dy = my;

                    LockLayerInfo(li);
                    moveLayer(v->WLayer, dx, dy);
                    UnlockLayerInfo(li);

                    BltBitMap(w->WScreen->RastPort.BitMap, v->LeftEdge, v->TopEdge, w->WScreen->RastPort.BitMap, v->LeftEdge + dx, v->TopEdge + dy, v->Width, v->Height, 0xc0, 0xff, NULL);

                    lock = LockIBase(0);
                    v->LeftEdge += dx;
                    v->TopEdge += dy;
                    UnlockIBase(lock);

                    WORD color = 1;

                    for (l = li->top_layer; l != NULL; l = l->back)
                    {
                        if (l->Flags & LAYERREFRESH)
                        {
                            if (!BeginUpdate(l))
                                printf("ERROR!\n");
                            else
                            {
                                GetRPAttrs(l->rp, RPTAG_DrawBounds, &rect, TAG_DONE);

                                SetRast(l->rp, color);
                                color++;
                            }

                            EndUpdate(l, TRUE);
                        }
                    }
                }
            }
        }
        CloseWindow(v);
    }
}

void openWindow(struct Screen *s)
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
        WA_SimpleRefresh,   TRUE,
        TAG_DONE))
    {
        openWindow2(w);
        CloseWindow(w);
    }
}

void openScreen(void)
{
    struct Screen *s;

    if (s = OpenScreenTags(NULL,
        SA_LikeWorkbench, TRUE,
        SA_Title,         "Layers ClipRects Test",
        TAG_DONE))
    {
        openWindow(s);
        CloseScreen(s);
    }
}

int main(void)
{
    openScreen();
    return 0;
}
