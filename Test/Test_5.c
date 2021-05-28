
#include <intuition/intuition.h>
#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>
#include <exec/memory.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include "MyClasses.h"
#include "IFF.h"
#include "GUI.h"

struct BitMap *gfx;

void drawBoard(struct BitMap *bm)
{
    WORD x, y;

    for (y = 0; y < BOARD_HEIGHT; y++)
    {
        for (x = 0; x < BOARD_WIDTH; x++)
        {
            if (x == 0 || x == BOARD_WIDTH - 1 || y == 0 || y == BOARD_HEIGHT - 1)
                BltBitMap(gfx, 5 << 4, 0, bm, x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);
            else
                BltBitMap(gfx, 7 << 4, 0, bm, x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);
        }
    }
}

BOOL createImages(struct boardInfo *bi)
{
    WORD i;
    Point points[] = { 0, 0, 0, 16, 0, 16 };

    for (i = 0; i < GID_COUNT; i++)
    {
        bi->images[i] = NewObject(bi->cl, NULL,
            IA_Width, 16,
            IA_Height, 16,
            IA_BitMap, gfx,
            IA_Points, points,
            TAG_DONE);

        points[0].x += 16;
        points[1].x += 16;
    }
    return(TRUE);
}

BOOL createGadgets(struct boardInfo *bi)
{
    WORD i;

    for (i = 0; i < GID_COUNT; i++)
    {
        bi->gadgets[i] = NewObject(NULL, "buttongclass",
            GA_Left, i << 4,
            GA_Top, 0,
            GA_Image, bi->images[i],
            GA_Width, 16,
            GA_Height, 16,
            GA_ID, i,
            /* i >= GID_HERO ? GA_ToggleSelect : TAG_IGNORE, TRUE, */
            i == GID_WALL ? GA_Selected : TAG_IGNORE, TRUE,
            i > 0 ? GA_Previous : TAG_IGNORE, bi->gadgets[i - 1],
            GA_Immediate, TRUE,
            GA_RelVerify, TRUE,
            TAG_DONE);
    }
    return(TRUE);
}

void disposeGadgets(struct boardInfo *bi)
{
    WORD i;

    for (i = 0; i < GID_COUNT; i++)
    {
        DisposeObject(bi->gadgets[i]);
    }
}

void disposeImages(struct boardInfo *bi)
{
    WORD i;

    for (i = 0; i < GID_COUNT; i++)
    {
        DisposeObject(bi->images[i]);
    }
}

int main()
{
    Class *cl;
    struct ColorMap *cm;
    BOOL mask;
    struct screenInfo si;
    struct boardInfo bi;
    ULONG *pal;

    if (gfx = loadBitMap("Dane/Sceneria1.iff", &cm, BMF_INTERLEAVED, &mask))
    {
        if (bi.cl = cl = makeImage())
        {
            struct Window *w;

            if (pal = getPalette(cm))
            {
                if (w = openScreen(&si, pal, drawBoard, &bi))
                {
                    struct DrawInfo *dri;
                    if (dri = GetScreenDrawInfo(w->WScreen))
                    {
                        if (createImages(&bi))
                        {
                            if (createGadgets(&bi))
                            {
                                AddGList(w, (struct Gadget *)bi.gadgets[0], -1, GID_COUNT, NULL);
                                RefreshGadgets((struct Gadget *)bi.gadgets[0], w, NULL);
                                BOOL done = FALSE;
                                while (!done)
                                {
                                    WORD cur = T_WALL;
                                    WaitPort(w->UserPort);
                                    struct IntuiMessage *msg;
                                    while (msg = (struct IntuiMessage *)GetMsg(w->UserPort))
                                    {
                                        if (msg->Class == IDCMP_RAWKEY)
                                            done = TRUE;
                                        else if (msg->Class == IDCMP_GADGETDOWN)
                                        {
                                            struct Gadget *gad = (struct Gadget *)msg->IAddress;
                                            printf("DOWN %d\n", gad->GadgetID);
                                            if (gad->GadgetID >= GID_HERO)
                                            {
                                                printf("Setting\n");
                                            }
                                        }
                                        else if (msg->Class == IDCMP_GADGETUP)
                                        {
                                            struct Gadget *gad = (struct Gadget *)msg->IAddress;
                                            printf("UP %d\n", gad->GadgetID);
                                        }
                                        ReplyMsg((struct Message *)msg);
                                    }
                                }

                                RemoveGList(w, (struct Gadget *)bi.gadgets[0], GID_COUNT);
                                disposeGadgets(&bi);
                            }
                            disposeImages(&bi);
                        }
                        FreeScreenDrawInfo(w->WScreen, dri);
                    }
                    closeScreen(w);
                }
                FreeVec(pal);
            }
            FreeClass(cl);
        }
        unloadBitMap(gfx, cm);
    }
    return(0);
}
