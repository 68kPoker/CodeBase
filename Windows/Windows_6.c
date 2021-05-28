
#include <intuition/intuition.h>
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <graphics/rpattr.h>
#include <datatypes/pictureclass.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/datatypes_protos.h>

#define ESC_KEY 0x45

#define HEIGHT 240
#define IDCMP  IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_REFRESHWINDOW|IDCMP_ACTIVEWINDOW

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

__far extern struct Custom custom;

/* Open game window context */

struct Window *openWindow(struct Screen *s)
{
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            0,
        WA_Top,             s->BarHeight + 1,
        WA_Width,           s->Width,
        WA_Height,          HEIGHT,
        WA_Borderless,      TRUE,
        WA_RMBTrap,         TRUE,
        WA_Activate,        TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,           IDCMP,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_ReportMouse,     TRUE,
        WA_ScreenTitle,     "Warehouse (C)2018-2020 Robert Szacki",
        TAG_DONE))
    {
        return w;
    }
    return NULL;
}

/* Alloc friend bitmap */

struct BitMap *allocFriend(struct Window *w)
{
    struct BitMap *bm, *friend = w->RPort->BitMap;

    if (bm = AllocBitMap(w->Width, w->Height, GetBitMapAttr(friend, BMA_DEPTH), BMF_INTERLEAVED, NULL))
    {
        return bm;
    }
    return NULL;
}

/* Clear bitmap + draw frame */

void clearBitMap(struct BitMap *bm, struct BitMap *gfx)
{
    struct RastPort rp;

    InitRastPort(&rp);
    rp.BitMap = bm;

    WORD x, y;

/*
    SetRPAttrs(&rp,
        RPTAG_APen,         20,
        RPTAG_OutlinePen,   27,
        TAG_DONE);

    RectFill(&rp, 0, 0, GetBitMapAttr(bm, BMA_WIDTH) - 1, GetBitMapAttr(bm, BMA_HEIGHT) - 1);
*/
    BltBitMap(gfx, 0, 0, bm, 0, 0, 640, 16, 0xc0, 0xff, NULL);
    for (y = 1; y < 15; y++)
    {
        for (x = 0; x < 20; x++)
        {
            WORD tile = 0;
            if (x == 0 || x == 19 || y == 0 || y == 14)
                tile = 1;
            BltBitMap(gfx, tile << 5, 32, bm, x << 5, y << 4, 32, 16, 0xc0, 0xff, NULL);
        }
    }
}

/* Draw logo */

void drawText(struct BitMap *bm, WORD x, WORD y, STRPTR text)
{
    struct RastPort rp;

    InitRastPort(&rp);
    rp.BitMap = bm;

    SetRPAttrs(&rp,
        RPTAG_APen,         6,
        RPTAG_DrMd,         JAM1,
        TAG_DONE);

    Move(&rp, x, y + rp.Font->tf_Baseline);
    Text(&rp, text, strlen(text));
}

/* Clear window */

void fastClear(struct Window *w, struct BitMap *bm)
{
    struct Custom *cust = &custom;
    struct BitMap *dest = w->RPort->BitMap;
    WORD bpr = bm->BytesPerRow, destbpr = dest->BytesPerRow;

    OwnBlitter();
    WaitBlit();

    cust->bltcon0 = SRCA | DEST | A_TO_D;
    cust->bltcon1 = 0;
    cust->bltapt  = bm->Planes[0];
    cust->bltdpt  = dest->Planes[0] + (w->TopEdge * destbpr);
    cust->bltamod = 0;
    cust->bltdmod = 0;
    cust->bltafwm = 0xffff;
    cust->bltalwm = 0xffff;
    cust->bltsizv = w->Height * GetBitMapAttr(dest, BMA_DEPTH);
    cust->bltsizh = w->Width >> 4;

    DisownBlitter();
}

/* Update portion */

void update(struct Window *w, struct BitMap *bm, WORD sx, WORD sy, WORD x, WORD y, WORD width, WORD height)
{
    struct Custom *cust = &custom;
    struct BitMap *dest = w->RPort->BitMap;
    WORD bpr = bm->BytesPerRow, destbpr = dest->BytesPerRow;
    UBYTE depth = GetBitMapAttr(dest, BMA_DEPTH);

    x >>= 4;
    sx >>= 4;
    width >>= 4;

    OwnBlitter();
    WaitBlit();

    cust->bltcon0 = SRCA | DEST | A_TO_D;
    cust->bltcon1 = 0;
    cust->bltapt  = bm->Planes[0] + (sy * bpr) + (sx << 1);
    cust->bltdpt  = dest->Planes[0] + ((w->TopEdge + y) * destbpr) + (x << 1);
    cust->bltamod = (bpr / depth) - (width << 1);
    cust->bltdmod = (destbpr / depth) - (width << 1);
    cust->bltafwm = 0xffff;
    cust->bltalwm = 0xffff;
    cust->bltsizv = height * depth;
    cust->bltsizh = width;

    DisownBlitter();
}

/* Update portion */

void updateBob(struct Window *w, struct BitMap *bm, WORD sx, WORD sy, WORD x, WORD y, WORD width, WORD height)
{
    struct Custom *cust = &custom;
    struct BitMap *dest = w->RPort->BitMap;
    WORD bpr = bm->BytesPerRow, destbpr = dest->BytesPerRow;
    UBYTE depth = GetBitMapAttr(dest, BMA_DEPTH);

    UBYTE xshift = x & 0xf;

    x >>= 4;
    sx >>= 4;
    width >>= 4;
    width++;

    OwnBlitter();
    WaitBlit();

    cust->bltcon0 = SRCB | SRCC | DEST | 0xca | (xshift << ASHIFTSHIFT);
    cust->bltcon1 = xshift << BSHIFTSHIFT;
    cust->bltadat = 0xffff;
    cust->bltbpt  = bm->Planes[0] + (sy * bpr) + (sx << 1);
    cust->bltcpt  = dest->Planes[0] + ((w->TopEdge + y) * destbpr) + (x << 1);
    cust->bltdpt  = dest->Planes[0] + ((w->TopEdge + y) * destbpr) + (x << 1);
    cust->bltbmod = (bpr / depth) - (width << 1);
    cust->bltcmod = (destbpr / depth) - (width << 1);
    cust->bltdmod = (destbpr / depth) - (width << 1);
    cust->bltafwm = 0xffff;
    cust->bltalwm = 0x0000;
    cust->bltsizv = height * depth;
    cust->bltsizh = width;

    DisownBlitter();
}

BOOL obtainPens(struct ColorMap *cm)
{
    WORD i;
    WORD pen;

    for (i = 4; i < 28; i++)
    {
        /* Skip pointer colors */
        if (i >= 17 && i <= 19)
            continue;

        pen = ObtainPen(cm, i, RGB(0xaa), RGB(0xaa), RGB(0xaa), PENF_EXCLUSIVE);
        if (pen == -1)
        {
            printf("Couldn't alloc screen pens!\n");
            break;
        }
    }

    if (pen != -1)
    {
        return(TRUE);
    }

    while (i > 4)
    {
        ReleasePen(cm, --i);
    }
    return(FALSE);
}

void releasePens(struct ColorMap *cm)
{
    WORD i;


    for (i = 27; i >= 4; i--)
    {
        if (i >= 17 && i <= 19)
            continue;
        ReleasePen(cm, i);
    }
}

Object *loadGraphics(STRPTR name, struct Screen *s, struct BitMap **bm)
{
    Object *o;
    if (o = NewDTObject(name,
        DTA_GroupID,    GID_PICTURE,
        PDTA_Screen,    s,
        PDTA_Remap,     FALSE,
        TAG_DONE))
    {
        WORD i;
        ULONG *cregs;
        struct ViewPort *vp = &s->ViewPort;
        GetDTAttrs(o, PDTA_CRegs, &cregs, TAG_DONE);
        for (i = 4; i < 32; i++)
        {
            if (i >= 17 && i <= 19)
                continue;
            SetRGB32(vp, i, cregs[(i * 3) + 0], cregs[(i * 3) + 1], cregs[(i * 3) + 2]);
        }
        DoDTMethod(o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);
        GetDTAttrs(o, PDTA_BitMap, bm, TAG_DONE);
        return(o);
    }
    return(NULL);
}

void loop(struct Window *w, struct BitMap *bm, struct BitMap *gfx)
{
    BOOL done = FALSE;
    WORD tile = 2;

    while (!done)
    {
        struct IntuiMessage *msg;
        WaitPort(w->UserPort);

        while (msg = GT_GetIMsg(w->UserPort))
        {
            if (msg->Class == IDCMP_ACTIVEWINDOW)
            {
                WindowToFront(w);
            }
            else if (msg->Class == IDCMP_RAWKEY)
            {
                if (msg->Code == ESC_KEY)
                {
                    done = TRUE;
                }
            }
            else if (msg->Class == IDCMP_MOUSEBUTTONS)
            {
                if (msg->Code == IECODE_LBUTTON)
                {
                    BltBitMap(gfx, tile << 5, 32, w->RPort->BitMap, msg->MouseX & 0xffe0, w->TopEdge + (msg->MouseY & 0xfff0), 32, 16, 0xc0, 0xff, NULL);
                }
            }
            GT_ReplyIMsg(msg);
        }
    }
}

int main()
{
    struct Screen *s;
    struct RastPort rp;
    struct BitMap *gfx;
    Object *o;
    struct Task *task;

    task = FindTask(NULL);
    SetTaskPri(task, 1);

    if (s = LockPubScreen(NULL))
    {
        if (obtainPens(s->ViewPort.ColorMap))
        {
            struct Window *w;

            if (w = openWindow(s))
            {
                struct BitMap *bm;
                if (bm = allocFriend(w))
                {
                    WORD i;

                    if (o = loadGraphics("Graphics.iff", s, &gfx))
                    {
                        clearBitMap(bm, gfx);
                        /* drawText(bm, 17, 2, "Warehouse Sokoban Game"); */
                        fastClear(w, bm);

                        InitRastPort(&rp);
                        rp.BitMap = bm;
                        loop(w, bm, gfx);


                        DisposeDTObject(o);
                    }
                    FreeBitMap(bm);
                }
                CloseWindow(w);
            }
            releasePens(s->ViewPort.ColorMap);
        }
        UnlockPubScreen(NULL, s);
    }
    SetTaskPri(task, 0);
    return 0;
}
