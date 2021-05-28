
/* Warehouse - game engine */

#include <intuition/intuition.h>
#include <datatypes/pictureclass.h>
#include <clib/intuition_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#define IDCMP IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_RAWKEY|IDCMP_INTUITICKS|IDCMP_MENUPICK

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 5

BOOL genMask(struct graphics *gfx);

struct Graphics
{
    struct BitMap *bm;
    PLANEPTR mask;
    WORD width, height;
    UWORD *pens; /* Screen pens */
    WORD tile; /* Current tile */
};

UWORD pens[] = { ~0 };

/* Init engine - obtains window */

LONG init(struct Window *w)
{
    struct RastPort *rp = w->RPort;
    struct Graphics *gfx = (struct Graphics *)rp->RP_User;

    /* Draw board here etc. */

    WORD x, y, z;

    for (z = 0; z < 2; z++)
    {
        for (x = z; x < 20; x += 2)
        {
            for (y = 0; y < 16; y++)
            {
                WORD srcx = 0, srcy = 16;

                if (x == 0 || x == 19 || y == 0 || y == 15)
                    srcx = 16;

                BltBitMapRastPort(gfx->bm, srcx, srcy, rp, x << 4, y << 4, 16, 16, 0xc0);
            }
            WaitTOF();
        }
    }

    return 0;
}

/* Engine - Obtains IntuiMessage */

LONG engine(struct IntuiMessage *msg)
{
    struct Window *w = msg->IDCMPWindow;
    struct RastPort *rp = w->RPort;
    struct Graphics *gfx = (struct Graphics *)rp->RP_User;

    switch (msg->Class)
    {
        case IDCMP_MOUSEBUTTONS:
            switch (msg->Code)
            {
                case IECODE_LBUTTON:
                    WORD srcx = gfx->tile % 20;
                    WORD srcy = (gfx->tile / 20) + 1;
                    WORD x = msg->MouseX >> 4;
                    WORD y = msg->MouseY >> 4;
                    BltBitMapRastPort(gfx->bm, srcx << 4, srcy << 4, rp, x << 4, y << 4, 16, 16, 0xc0);
                    break;

                case IECODE_RBUTTON:
                    gfx->tile++;
                    gfx->tile %= 9;
                    break;
            }
            return 1;

        case IDCMP_RAWKEY:
            /* Raw key */
            if (msg->Code == 0x45)
                return 0;
            return 1;

        case IDCMP_MENUPICK:
            /* Menu pick */
            return 1;

        case IDCMP_INTUITICKS:
            static WORD frame = 0;
            static WORD pos = 1;
            BltBitMapRastPort(gfx->bm, 7 << 4, 1 << 4, rp, pos << 4, 5 << 4, 16, 16, 0xc0);
            BltMaskBitMapRastPort(gfx->bm, frame << 4, 6 << 4, rp, pos << 4, 5 << 4, 16, 16, ABC|ABNC|ANBC, gfx->mask);
            frame++;
            if (frame == 6)
            {
                BltBitMapRastPort(gfx->bm, 7 << 4, 1 << 4, rp, pos << 4, 5 << 4, 16, 16, 0xc0);
                pos++;
                frame = 0;
            }
            return 1;
    }
    return -1;
}

struct Window *openWindow(struct Graphics *gfx)
{
    struct Screen *s;

    if (s = OpenScreenTags(NULL,
        SA_Left,        0,
        SA_Top,         0,
        SA_Width,       WIDTH,
        SA_Height,      HEIGHT,
        SA_Depth,       DEPTH,
        SA_DisplayID,   LORES_KEY,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_ShowTitle,   FALSE,
        SA_Pens,        pens,
        TAG_DONE))
    {
        struct Window *w;

        if (w = OpenWindowTags(NULL,
            WA_CustomScreen,    s,
            WA_Left,            0,
            WA_Top,             0,
            WA_Width,           s->Width,
            WA_Height,          s->Height,
            WA_CloseGadget,     FALSE,
            WA_DragBar,         FALSE,
            WA_DepthGadget,     FALSE,
            WA_Borderless,      TRUE,
            WA_Activate,        TRUE,
            WA_RMBTrap,         TRUE,
            WA_SimpleRefresh,   TRUE,
            WA_BackFill,        LAYERS_NOBACKFILL,
            WA_IDCMP,           IDCMP,
            WA_GimmeZeroZero,   TRUE,
            WA_ReportMouse,     TRUE,
            TAG_DONE))
        {
            Object *o;
            if (o = NewDTObject("Data/Magazyn.pic",
                DTA_GroupID,    GID_PICTURE,
                PDTA_Screen,    s,
                PDTA_Remap,     FALSE,
                TAG_DONE))
            {
                ULONG *cregs, numcolors;
                struct BitMap *bm;

                DoDTMethod(o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);
                GetDTAttrs(o,
                    PDTA_BitMap, &bm,
                    PDTA_CRegs, &cregs,
                    PDTA_NumColors, &numcolors,
                    TAG_DONE);

                WORD i;

                for (i = 0; i < numcolors; i++)
                {
                    SetRGB32CM(s->ViewPort.ColorMap, i, cregs[0], cregs[1], cregs[2]);
                    cregs += 3;
                }
                MakeScreen(s);
                RethinkDisplay();

                w->UserData = (APTR)o;
                gfx->bm = bm;
                w->RPort->RP_User = (APTR)gfx;
                gfx->tile = 4;

                if (genMask(gfx))
                {
                    WaitBlit();
                    DisposeDTObject(o);
                    return w;
                }
                DisposeDTObject(o);
            }
            CloseWindow(w);
        }
        CloseScreen(s);
    }
    return NULL;
}

BOOL genMask(struct Graphics *gfx)
{
    struct BitMap *bm = gfx->bm, aux;
    WORD width = GetBitMapAttr(bm, BMA_WIDTH);
    WORD height = GetBitMapAttr(bm, BMA_HEIGHT);
    WORD depth = GetBitMapAttr(bm, BMA_DEPTH);

    printf("%d * %d * %d\n", width, height, depth);

    if (bm = AllocBitMap(width, height, depth, 0, NULL))
    {
        BltBitMap(gfx->bm, 0, 0, bm, 0, 0, width, height, 0xc0, 0xff, NULL);

        if (gfx->mask = AllocRaster(width, height))
        {
            WORD i;

            InitBitMap(&aux, depth, width, height);
            for (i = 0; i < depth; i++)
            {
                aux.Planes[i] = gfx->mask;
            }

            BltBitMap(gfx->bm, 0, 0, &aux, 0, 0, width, height, 0xc0, 1, NULL);

            for (i = 1; i < depth; i++)
            {
                WORD result;
                result =  BltBitMap(gfx->bm, 0, 0, &aux, 0, 0, width, height, 0xe0, 1 << i, NULL);
            }
            gfx->bm = bm;
            gfx->width = width;
            gfx->height = height;
            return(TRUE);
        }
        FreeBitMap(bm);
    }
    return(FALSE);
}

void closeWindow(struct Window *w)
{
    struct Screen *s = w->WScreen;
    Object *o = (Object *)w->UserData;
    struct Graphics *gfx = (struct Graphics *)w->RPort->RP_User;

    FreeRaster(gfx->mask, gfx->width, gfx->height);
    FreeBitMap(gfx->bm);
    CloseWindow(w);
    CloseScreen(s);
}

int main()
{
    struct Window *w;
    struct MsgPort *mp;
    struct Graphics gfx;

    if (w = openWindow(&gfx))
    {
        mp = w->UserPort;
        BOOL done = FALSE;

        init(w);

        while (!done)
        {
            struct IntuiMessage *msg;
            WaitPort(mp);
            while ((!done) && (msg = (struct IntuiMessage *)GetMsg(mp)))
            {
                done = !engine(msg);
                ReplyMsg((struct Message *)msg);
            }
        }
        closeWindow(w);
    }
    return 0;
}
