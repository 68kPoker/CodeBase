
#include <exec/types.h>
#include <intuition/screens.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>

#define ESC_KEY 0x45

#define DRAW   0 /* dbuf index */
#define CHANGE 1

enum
{
    SIG_DRAW,
    SIG_CHANGE,
    SIG_IDCMP,
    SOURCES
};

struct screenInfo
{
    struct TextFont     *tf; /* Opened disk font */
    struct bitMapInfo
    {
        struct BitMap       *bm; /* Custom bitmap */
        struct ScreenBuffer *sb;
        struct Region       *update; /* Region changed */
    } bmi[2];
    struct Screen       *s;
    struct dBufInfo
    {
        struct MsgPort  *port; /* Port */
        BOOL            safe; /* Safe to draw/change? */
    } dbi[2];
    WORD            frame; /* Hidden frame */
    struct Window   *window; /* Backdrop window */
};

BOOL initScreen(struct screenInfo *si)
{
    struct TextAttr ta =
    {
        "helvetica.font", 9,
        FS_NORMAL,
        FPF_DISKFONT | FPF_DESIGNED
    };

    WORD dispWidth  = 320;
    WORD dispHeight = 256;
    UBYTE depth = 5;
    ULONG modeID = LORES_KEY;
    UBYTE title[] = "Warehouse";
    ULONG idcmpFlags = IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE;

    WORD rasWidth  = dispWidth;
    WORD rasHeight = dispHeight;
    if (si->tf = OpenDiskFont(&ta))
    {
        if (si->bmi[0].bm = AllocBitMap(rasWidth, rasHeight, depth, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
        {
            if (si->s = OpenScreenTags(NULL,
                SA_Font,    &ta,
                SA_BitMap,  si->bmi[0].bm,
                SA_Left,    0,
                SA_Top,     0,
                SA_Width,   dispWidth,
                SA_Height,  dispHeight,
                SA_Depth,   depth,
                SA_DisplayID,   modeID,
                SA_Quiet,   TRUE,
                SA_ShowTitle,   FALSE,
                SA_Title,   title,
                SA_BackFill, LAYERS_NOBACKFILL,
                TAG_DONE))
            {
                if (si->dbi[DRAW].port = CreateMsgPort())
                {
                    si->dbi[DRAW].safe = TRUE;
                    if (si->dbi[CHANGE].port = CreateMsgPort())
                    {
                        si->dbi[CHANGE].safe = TRUE;
                        if (si->bmi[0].sb = AllocScreenBuffer(si->s, si->bmi[0].bm, 0))
                        {
                            if (si->bmi[1].bm = AllocBitMap(rasWidth, rasHeight, depth, BMF_DISPLAYABLE, NULL))
                            {
                                if (si->bmi[1].sb = AllocScreenBuffer(si->s, si->bmi[1].bm, 0))
                                {
                                    si->frame = 1;
                                    si->bmi[0].sb->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = si->dbi[DRAW].port;
                                    si->bmi[1].sb->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = si->dbi[DRAW].port;
                                    si->bmi[0].sb->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = si->dbi[CHANGE].port;
                                    si->bmi[1].sb->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = si->dbi[CHANGE].port;
                                    if (si->window = OpenWindowTags(NULL,
                                        WA_CustomScreen,    si->s,
                                        WA_Left,            0,
                                        WA_Top,             0,
                                        WA_Width,           si->s->Width,
                                        WA_Height,          si->s->Height,
                                        WA_Backdrop,        TRUE,
                                        WA_Borderless,      TRUE,
                                        WA_Activate,        TRUE,
                                        WA_RMBTrap,         TRUE,
                                        WA_SimpleRefresh,   TRUE,
                                        WA_BackFill,        LAYERS_NOBACKFILL,
                                        WA_IDCMP,           idcmpFlags,
                                        WA_ReportMouse,     TRUE,
                                        TAG_DONE))
                                    {
                                        return(TRUE);
                                    }
                                    FreeScreenBuffer(si->s, si->bmi[1].sb);
                                }
                                FreeBitMap(si->bmi[1].bm);
                            }
                            FreeScreenBuffer(si->s, si->bmi[0].sb);
                        }
                        DeleteMsgPort(si->dbi[CHANGE].port);
                    }
                    DeleteMsgPort(si->dbi[DRAW].port);
                }
                CloseScreen(si->s);
            }
            FreeBitMap(si->bmi[0].bm);
        }
        CloseFont(si->tf);
    }
    return(FALSE);
}

void loopScreen(struct screenInfo *si)
{
    ULONG signals[SOURCES] = { 0 }, total = 0;
    BOOL done = FALSE;

    signals[SIG_DRAW] = 1L << si->dbi[DRAW].port->mp_SigBit;
    signals[SIG_CHANGE] = 1L << si->dbi[CHANGE].port->mp_SigBit;
    signals[SIG_IDCMP] = 1L << si->window->UserPort->mp_SigBit;

    while (!done)
    {
        ULONG result;
        total = signals[SIG_DRAW] | signals[SIG_CHANGE] | signals[SIG_IDCMP];
        result = Wait(total);

        if (result & signals[SIG_DRAW])
        {
            /* Safe to draw */
            if (!si->dbi[DRAW].safe)
            {
                while (!si->dbi[DRAW].port)
                {
                    WaitPort(si->dbi[DRAW].port);
                }
                si->dbi[DRAW].safe = TRUE;
            }
        }
        if (result & signals[SIG_CHANGE])
        {
            /* Safe to draw */
            if (!si->dbi[CHANGE].safe)
            {
                while (!si->dbi[CHANGE].port)
                {
                    WaitPort(si->dbi[CHANGE].port);
                }
                si->dbi[CHANGE].safe = TRUE;
            }
        }
        if (result & signals[SIG_IDCMP])
        {
            /* IDCMP */
            struct MsgPort *mp = si->window->UserPort;
            struct IntuiMessage *msg;
            while (msg = (struct IntuiMessage *)GetMsg(mp))
            {
                ULONG class = msg->Class;
                UWORD code  = msg->Code;
                WORD  mx    = msg->MouseX;
                WORD  my    = msg->MouseY;
                APTR  iaddr = msg->IAddress;

                ReplyMsg((struct Message *)msg);

                if (class == IDCMP_RAWKEY)
                {
                    if (code == ESC_KEY)
                    {
                        done = TRUE;
                    }
                }
            }
        }
    }
}

void closeScreen(struct screenInfo *si)
{
    CloseWindow(si->window);
    if (!si->dbi[CHANGE].safe)
    {
        while (!si->dbi[CHANGE].port)
        {
            WaitPort(si->dbi[CHANGE].port);
        }
    }
    if (!si->dbi[DRAW].safe)
    {
        while (!si->dbi[DRAW].port)
        {
            WaitPort(si->dbi[DRAW].port);
        }
    }
    FreeScreenBuffer(si->s, si->bmi[1].sb);
    FreeBitMap(si->bmi[1].bm);
    FreeScreenBuffer(si->s, si->bmi[0].sb);
    DeleteMsgPort(si->dbi[CHANGE].port);
    DeleteMsgPort(si->dbi[DRAW].port);
    CloseScreen(si->s);
    FreeBitMap(si->bmi[0].bm);
    CloseFont(si->tf);
}
