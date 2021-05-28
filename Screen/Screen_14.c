
#include <intuition/screens.h>
#include <graphics/gfxmacros.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#include "Screen.h"

#define COMMANDS 3 /* Copperlist commands */

extern void myCopper();
extern __far struct Custom custom;

/* Additional screen data */
struct screenInfo
{
    struct Screen       *s;
    struct ScreenBuffer *sb[2];
    struct MsgPort      *safeport;
    BOOL                safe;
    UWORD               frame;
    struct Interrupt    copis;
    struct copperInfo
    {
        struct ViewPort *vp;
        UWORD           signal;
        struct Task     *task;
    } cop;
};

/* Open our game screen */
BOOL openScreen(struct screenInfo *si)
{
    if (si->s = OpenScreenTags(NULL,
        SA_Left,        0,
        SA_Top,         0,
        SA_Width,       320,
        SA_Height,      256,
        SA_Depth,       5,
        SA_DisplayID,   LORES_KEY,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_Exclusive,   TRUE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        si->s->UserData = (APTR)si;
        return(TRUE);
    }
    return(FALSE);
}

/* Add double-buffering support */
BOOL addDBuf(struct screenInfo *si)
{
    if (si->sb[0] = AllocScreenBuffer(si->s, NULL, SB_SCREEN_BITMAP))
    {
        if (si->sb[1] = AllocScreenBuffer(si->s, NULL, 0))
        {
            if (si->safeport = CreateMsgPort())
            {
                si->sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = si->safeport;
                si->sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = si->safeport;
                si->safe = TRUE;
                si->frame = 1;
                return(TRUE);
            }
            FreeScreenBuffer(si->s, si->sb[1]);
        }
        FreeScreenBuffer(si->s, si->sb[0]);
    }
    return(FALSE);
}

/* Free double-buffering support */
void freeDBuf(struct screenInfo *si)
{
    if (!si->safe)
    {
        while (!GetMsg(si->safeport))
        {
            WaitPort(si->safeport);
        }
    }
    DeleteMsgPort(si->safeport);
    FreeScreenBuffer(si->s, si->sb[1]);
    FreeScreenBuffer(si->s, si->sb[0]);
}

/* Add copperlist support */
BOOL addCopper(struct screenInfo *si)
{
    si->copis.is_Code = myCopper;
    si->copis.is_Data = (APTR)&si->cop;
    si->copis.is_Node.ln_Pri = 0;
    si->copis.is_Node.ln_Name = "Gear Works";

    si->cop.vp = &si->s->ViewPort;
    si->cop.task = FindTask(NULL);
    if ((si->cop.signal = AllocSignal(-1)) != -1)
    {
        struct UCopList *ucl;

        if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
        {
            CINIT(ucl, COMMANDS);
            CWAIT(ucl, 0, 0);
            CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
            CEND(ucl);

            Forbid();
            si->s->ViewPort.UCopIns = ucl;
            Permit();

            AddIntServer(INTB_COPER, &si->copis);
            return(TRUE);
        }
        FreeSignal(si->cop.signal);
    }
    return(FALSE);
}

/* Free copperlist support */
void freeCopper(struct screenInfo *si)
{
    RemIntServer(INTB_COPER, &si->copis);
    FreeSignal(si->cop.signal);
}

/* Grouping all together */
struct Screen *openGameScreen()
{
    struct screenInfo *si;

    if (si = AllocMem(sizeof(*si), MEMF_PUBLIC|MEMF_CLEAR))
    {
        if (openScreen(si))
        {
            if (addDBuf(si))
            {
                if (addCopper(si))
                {
                    return(si->s);
                }
                freeDBuf(si);
            }
            CloseScreen(si->s);
        }
        FreeMem(si, sizeof(*si));
    }
    return(NULL);
}

/* Close all */
void closeGameScreen(struct Screen *s)
{
    struct screenInfo *si = (struct screenInfo *)s->UserData;

    freeCopper(si);
    freeDBuf(si);
    CloseScreen(s);
    FreeMem(si, sizeof(*si));
}

/* Obtain safe signal */
ULONG safeSignal(struct Screen *s)
{
    struct screenInfo *si = (struct screenInfo *)s->UserData;

    return(1L << si->safeport->mp_SigBit);
}

/* Obtain copper signal */
ULONG copperSignal(struct Screen *s)
{
    struct screenInfo *si = (struct screenInfo *)s->UserData;

    return(1L << si->cop.signal);
}

/* Wait till it's safe to write */
void safeToWrite(struct Screen *s)
{
    struct screenInfo *si = (struct screenInfo *)s->UserData;

    if (!si->safe)
    {
        while (!GetMsg(si->safeport))
        {
            WaitPort(si->safeport);
        }
        si->safe = TRUE;
    }
}

/* Obtain current bitmap and frame */
struct BitMap *getBitMap(struct Screen *s, UWORD *frame)
{
    struct screenInfo *si = (struct screenInfo *)s->UserData;

    safeToWrite(s);
    *frame = si->frame;
    return(si->sb[si->frame]->sb_BitMap);
}

/* Swap buffers */
void changeBitMap(struct Screen *s)
{
    struct screenInfo *si = (struct screenInfo *)s->UserData;
    UWORD frame = si->frame;

    WaitBlit();
    while (!ChangeScreenBuffer(s, si->sb[frame]))
    {
        WaitTOF();
    }

    si->frame = frame ^= 1;
    si->safe = FALSE;
}
