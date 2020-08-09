
/* Screen and windows */

#include "ScrnWin.h"

#include <intuition/intuition.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

BOOL openScreen(struct screenUser *su)
{
    const WORD width = 320, height = 256, depth = 5;
    const WORD screenWidth = 320, screenHeight = 256;
    const ULONG modeID = LORES_KEY;
    const ULONG backFill = (ULONG)LAYERS_NOBACKFILL;
    const ULONG flags = BMF_CLEAR;

    if (su->bm[0] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|flags, NULL))
    {
        if (su->bm[1] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|flags, NULL))
        {
            if (su->s = OpenScreenTags(NULL,
                SA_Left,        0,
                SA_Top,         0,
                SA_Width,       screenWidth,
                SA_Height,      screenHeight,
                SA_Depth,       depth,
                SA_DisplayID,   modeID,
                SA_Quiet,       TRUE,
                SA_Exclusive,   TRUE,
                SA_ShowTitle,   FALSE,
                SA_Draggable,   FALSE,
                SA_BackFill,    backFill,
                SA_BitMap,      su->bm[0],
                TAG_DONE))
            {
                if (su->mp[0] = CreateMsgPort())
                {
                    if (su->mp[1] = CreateMsgPort())
                    {
                        if (su->dbi = AllocDBufInfo(&su->s->ViewPort))
                        {
                            su->safe[0] = su->safe[1] = TRUE;
                            su->dbi->dbi_SafeMessage.mn_ReplyPort = su->mp[0];
                            su->dbi->dbi_DispMessage.mn_ReplyPort = su->mp[1];
                            su->frame = 1;

                            if (initRastPort(&su->rpu, &su->s->RastPort))
                            {
                                su->s->UserData = (APTR)su;

                                return(TRUE);
                            }
                            FreeDBufInfo(su->dbi);
                        }
                        DeleteMsgPort(su->mp[1]);
                    }
                    DeleteMsgPort(su->mp[0]);
                }
                CloseScreen(su->s);
            }
            FreeBitMap(su->bm[1]);
        }
        FreeBitMap(su->bm[0]);
    }
    return(FALSE);
}

void safeToDraw(struct screenUser *su)
{
    if (!su->safe[0])
    {
        while (!GetMsg(su->mp[0]))
            WaitPort(su->mp[0]);
        su->safe[0] = TRUE;
    }
}

void safeToChange(struct screenUser *su)
{
    WORD frame = su->frame;

    if (!su->safe[1])
    {
        while (!GetMsg(su->mp[1]))
            WaitPort(su->mp[1]);
        su->safe[1] = TRUE;
    }

    ChangeVPBitMap(&su->s->ViewPort, su->bm[frame], su->dbi);
    su->frame = frame ^ 1;
    su->safe[0] = su->safe[1] = FALSE;
}

void closeScreen(struct screenUser *su)
{
    freeRastPort(&su->rpu);

    if (!su->safe[1])
        while (!GetMsg(su->mp[1]))
            WaitPort(su->mp[1]);

    if (!su->safe[0])
        while (!GetMsg(su->mp[0]))
            WaitPort(su->mp[0]);

    FreeDBufInfo(su->dbi);
    DeleteMsgPort(su->mp[1]);
    DeleteMsgPort(su->mp[0]);
    CloseScreen(su->s);
    FreeBitMap(su->bm[1]);
    FreeBitMap(su->bm[0]);
}

struct Window *openWindow(struct screenUser *su)
{
    struct Window *w;
    const ULONG idcmp = IDCMP_RAWKEY;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    su->s,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           su->s->Width,
        WA_Height,          su->s->Height,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_SimpleRefresh,   TRUE,
        WA_IDCMP,           idcmp,
        TAG_DONE))
    {
        su->bdw = w;
        return(w);
    }
    return(NULL);
}

void animScreen(struct screenUser *su)
{
    ULONG signals[] =
    {
        1L << su->mp[0]->mp_SigBit,
        1L << su->mp[1]->mp_SigBit,
        1L << su->bdw->UserPort->mp_SigBit,
    }, total = signals[0] | signals[1] | signals[2];
    BOOL done = FALSE;

    safeToChange(su);

    while (!done)
    {
        ULONG result = Wait(total);

        if (result & signals[SSAFE])
        {
            safeToDraw(su);
            su->rpu.frame = su->frame;
            su->rpu.rp->BitMap = su->bm[su->frame];
            drawAnim(su->rpu.rp);
        }

        if (result & signals[SDISP])
        {
            safeToChange(su);
        }

        if (result & signals[SUSER])
        {
            done = TRUE;
        }
    }
}

int main()
{
    struct screenUser su;

    if (openScreen(&su))
    {
        if (openWindow(&su))
        {
            animScreen(&su);
            CloseWindow(su.bdw);
        }
        closeScreen(&su);
    }
    return(0);
}
