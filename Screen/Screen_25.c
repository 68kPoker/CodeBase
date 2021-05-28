
/* Screen */

#include <intuition/intuition.h>
#include <datatypes/pictureclass.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/datatypes_protos.h>

#define WIDTH  320
#define HEIGHT 256
#define DEPTH  5

#define BOARD_WIDTH  20
#define BOARD_HEIGHT 16

struct backFillInfo
{
    struct Layer *layer;
    struct Rectangle bounds;
    LONG XOffset, YOffset;
};

VOID fastBackFill(register __a0 struct Hook *hook, register __a2 struct RastPort *rp, register __a1 struct backFillInfo *bf);

struct Hook fastBackFillHook =
{
    { NULL, NULL },
    (ULONG(*)())fastBackFill,
    NULL,
    NULL
};

__far extern struct Custom custom;

VOID blitIcon(PLANEPTR src, PLANEPTR dest, WORD srcmod, WORD destmod, WORD width, WORD height)
{
    struct Custom *cust = &custom;

    WaitBlit();
    cust->bltcon0 = 0x09f0;
    cust->bltcon1 = 0x0000;
    cust->bltapt  = src;
    cust->bltdpt  = dest;
    cust->bltamod = srcmod;
    cust->bltdmod = destmod;
    cust->bltafwm = 0xffff;
    cust->bltalwm = 0xffff;
    cust->bltsize = (height << 6) | width;
}

/* Fast back filling with Blitter, assumes 16-pixel align */
__saveds VOID fastBackFill(register __a0 struct Hook *hook, register __a2 struct RastPort *rp, register __a1 struct backFillInfo *bf)
{
    struct BitMap *bm = rp->BitMap, *gfx = (struct BitMap *)hook->h_Data;
    WORD plane, depth = bm->Depth, bpr = bm->BytesPerRow;
    struct Rectangle *bounds = &bf->bounds;
    LONG offset = (bpr * bounds->MinY) + (bounds->MinX >> 3);
    LONG gfxoffset = (gfx->BytesPerRow * bf->YOffset) + (bf->XOffset >> 3);
    WORD width = (bounds->MaxX - bounds->MinX + 1) >> 4;
    WORD modulo = bpr - (width << 1);
    WORD gfxmodulo = gfx->BytesPerRow - (width << 1);
    WORD height  = bounds->MaxY - bounds->MinY + 1;

    OwnBlitter();

    for (plane = 0; plane < depth; plane++)
        blitIcon(gfx->Planes[plane] + gfxoffset, bm->Planes[plane] + offset, gfxmodulo, modulo, width, height);

    DisownBlitter();
}

ULONG bestModeID()
{
    struct Screen *s;

    if ((s = LockPubScreen(NULL)) != NULL) {
        ULONG modeID = BestModeID(
            BIDTAG_ViewPort,        &s->ViewPort,
            BIDTAG_NominalWidth,    WIDTH,
            BIDTAG_NominalHeight,   HEIGHT,
            BIDTAG_Depth,           DEPTH,
            TAG_DONE);

        UnlockPubScreen(NULL, s);
        return(modeID);
    }
    return(INVALID_ID);
}

struct Screen *openScreen()
{
    ULONG modeID;

    if ((modeID = bestModeID()) != INVALID_ID) {
        struct Screen *s;

        if (s = OpenScreenTags(NULL,
            SA_Left,        0,
            SA_Top,         0,
            SA_Width,       WIDTH,
            SA_Height,      HEIGHT,
            SA_Depth,       DEPTH,
            SA_DisplayID,   modeID,
            SA_Quiet,       TRUE,
            SA_Exclusive,   TRUE,
            SA_BackFill,    LAYERS_NOBACKFILL,
            SA_Draggable,   FALSE,
            SA_ShowTitle,   FALSE,
            SA_Title,       "Magazyn",
            TAG_DONE)) {
            return(s);
        }
        else
            printf("Couldn't open screen!\n");
    }
    else
        printf("Couldn't get screen mode!\n");
    return(NULL);
}

struct BitMap *prepBoard(struct BitMap *gfx)
{
    struct BitMap *bm;

    if (bm = AllocBitMap(WIDTH, HEIGHT, DEPTH, 0, NULL)) {
        WORD x, y;
        WORD srcmod = gfx->BytesPerRow - 2, destmod = bm->BytesPerRow - 2;
        LONG offset, gfxoffset;

        OwnBlitter();
        for (y = 0; y < BOARD_HEIGHT; y++) {
            for (x = 0; x < BOARD_WIDTH; x++) {
                if (x == 0 || x == BOARD_WIDTH - 1 || y == 0 || y == BOARD_HEIGHT - 1)
                    gfxoffset = 2;
                else
                    gfxoffset = gfx->BytesPerRow << 4;

                WORD plane;
                offset = (y * (bm->BytesPerRow << 4)) + (x << 1);
                for (plane = 0; plane < DEPTH; plane++)
                    blitIcon(gfx->Planes[plane] + gfxoffset, bm->Planes[plane] + offset, srcmod, destmod, 1, 16);
            }
        }
        DisownBlitter();
        return(bm);
    }
    return(NULL);
}

struct Window *openWindow(Object **optr, struct BitMap **gfxbm)
{
    struct Screen *s;

    if (s = openScreen()) {
        Object *o;

        if (o = NewDTObject("Dane/Grafika.iff",
            DTA_GroupID,    GID_PICTURE,
            PDTA_Screen,    s,
            PDTA_Remap,     FALSE,
            TAG_DONE)) {
            struct BitMap *bm;
            ULONG *cregs, numcolors;
            struct Window *w;
            struct ColorMap *cm = s->ViewPort.ColorMap;
            WORD col;

            DoDTMethod(o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE);
            GetDTAttrs(o,
                PDTA_BitMap,    &bm,
                PDTA_CRegs,     &cregs,
                PDTA_NumColors, &numcolors,
                TAG_DONE);

            for (col = 0; col < numcolors; col++) {
                SetRGB32CM(cm, col, cregs[0], cregs[1], cregs[2]);
                cregs += 3;
            }
            MakeScreen(s);
            RethinkDisplay();

            if (*gfxbm = prepBoard(bm)) {

                /* Pass parameter to Hook */
                fastBackFillHook.h_Data = (APTR)*gfxbm;

                if (w = OpenWindowTags(NULL,
                    WA_CustomScreen,    s,
                    WA_Left,            0,
                    WA_Top,             0,
                    WA_Width,           s->Width,
                    WA_Height,          s->Height,
                    WA_Backdrop,        FALSE,
                    WA_Borderless,      TRUE,
                    WA_Activate,        TRUE,
                    WA_RMBTrap,         TRUE,
                    WA_IDCMP,           IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS,
                    WA_BackFill,        &fastBackFillHook,
                    WA_SimpleRefresh,   TRUE,
                    TAG_DONE)) {
                    *optr = o;
                    return(w);
                }
                else
                    printf("Couldn't open window!\n");
                FreeBitMap(*gfxbm);
            }
            DisposeDTObject(o);
        }
        else
            printf("Couldn't load graphics!\n");
        CloseScreen(s);
    }
    return(NULL);
}

main()
{
    struct Window *w;
    Object *o;
    struct BitMap *gfxbm;

    if (w = openWindow(&o, &gfxbm)) {
        struct Screen *s = w->WScreen;

        WaitPort(w->UserPort);
        CloseWindow(w);
        FreeBitMap(gfxbm);
        DisposeDTObject(o);
        CloseScreen(s);
    }
    return(0);
}
