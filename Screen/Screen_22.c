
#include <intuition/intuition.h>
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <devices/timer.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include <clib/timer_protos.h>

extern __far struct Custom custom;

/* Alloc screen bitmap + raster bitmap */

struct BitMap *allocBitMaps(struct BitMap **rasbm, WORD width, WORD height, UBYTE depth, WORD raswidth, WORD rasheight)
{
    struct BitMap *bm;

    if (bm = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
    {
        if (*rasbm = AllocBitMap(raswidth, rasheight, depth, BMF_DISPLAYABLE|BMF_INTERLEAVED, NULL))
        {
            return(bm);
        }
        FreeBitMap(bm);
    }
    return(NULL);
}

struct Screen *openScreen(ULONG modeID, WORD width, WORD height, UBYTE depth, struct BitMap *bm)
{
    struct Screen *s;

    if (s = OpenScreenTags(NULL,
        SA_Left,        0,
        SA_Top,         0,
        SA_Width,       width,
        SA_Height,      height,
        SA_Depth,       depth,
        SA_DisplayID,   modeID,
        SA_BitMap,      bm,
        SA_Exclusive,   TRUE,
        SA_Draggable,   FALSE,
        SA_ShowTitle,   FALSE,
        SA_Quiet,       TRUE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        return(s);
    }
    return(NULL);
}

void drawIcon(struct BitMap *rasbm, WORD x, WORD y, ULONG signalmask)
{
    struct Custom *cust = &custom;

    OwnBlitter();
    WaitBlit();
    /* Wait(signalmask); */

    cust->bltcon0 = DEST|A_TO_D;
    cust->bltcon1 = 0;
    cust->bltadat = 0xffff;
    cust->bltdpt  = rasbm->Planes[0] + ((y * rasbm->BytesPerRow) << 4) + (x << 1);
    cust->bltamod = 0;
    cust->bltdmod = rasbm->Planes[1] - rasbm->Planes[0] - 2;
    cust->bltafwm = 0xffff;
    cust->bltalwm = 0xffff;
    cust->bltsizv = 16 * rasbm->Depth;
    cust->bltsizh = 1;

    DisownBlitter();
}

void test(struct Screen *s, struct BitMap *rasbm, ULONG signalmask)
{
    WORD i;
    struct RastPort *rp = &s->RastPort;
    struct EClockVal ecv1, ecv2;
    char text[5];

    SetABPenDrMd(rp, 1, 2, JAM2);

    ReadEClock(&ecv1);

    for (i = 0; i < 1280; i++)
    {
        drawIcon(rasbm, i % 40, i / 40, signalmask);
        if ((i % 64) == 63)
        {
            drawIcon(rp->BitMap, i >> 5, 0, signalmask);
            drawIcon(rp->BitMap, (i >> 5) - 1, 0, signalmask);
        }
    }

    ReadEClock(&ecv2);

    s->ViewPort.RasInfo->BitMap = rasbm;
    MakeScreen(s);
    RethinkDisplay();

    printf("%d\n", ecv2.ev_lo - ecv1.ev_lo);
}
