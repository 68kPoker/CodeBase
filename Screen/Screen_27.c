
/* Obsîuga ekranu */

/* $Log$ */

#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <graphics/gfxmacros.h>

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "Screen.h"

extern __far struct Custom custom;
extern void myCopper();

UWORD pens[] = { ~0 };

struct Screen *openScreen()
{
    struct Screen *s;
    SD sd;
    struct Rectangle dclip = { 0, 0, 319, 255 };

    if (sd = AllocMem(sizeof(*sd), MEMF_PUBLIC|MEMF_CLEAR))
    {
        if (sd->s = s = OpenScreenTags(NULL,
            SA_DClip,       &dclip,
            SA_DisplayID,   LORES_KEY,
            SA_Depth,       DEPTH,
            SA_Quiet,       TRUE,
            SA_Exclusive,   TRUE,
            SA_ShowTitle,   FALSE,
            SA_BackFill,    LAYERS_NOBACKFILL,
            SA_Title,       "Magazyn",
            SA_Interleaved, TRUE,
            SA_Pens,        pens,
            TAG_DONE))
        {
            s->UserData = (APTR)sd;
            if (sd->sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
            {
                if (sd->sb[1] = AllocScreenBuffer(s, NULL, 0))
                {
                    if (sd->mp = CreateMsgPort())
                    {
                        struct Interrupt *is = &sd->is;

                        sd->safe = TRUE;
                        sd->frame = 1;
                        sd->sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sd->mp;
                        sd->sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sd->mp;

                        is->is_Code = myCopper;
                        is->is_Data = (APTR)&sd->cop;
                        is->is_Node.ln_Pri = COP_PRI;

                        if ((sd->cop.signal = AllocSignal(-1)) != -1)
                        {
                            struct UCopList *ucl;

                            sd->cop.task = FindTask(NULL);

                            if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                            {
                                if (sd->w = OpenWindowTags(NULL,
                                    WA_CustomScreen,    s,
                                    WA_Left,            0,
                                    WA_Top,             0,
                                    WA_Width,           s->Width,
                                    WA_Height,          s->Height,
                                    WA_Backdrop,        TRUE,
                                    WA_Borderless,      TRUE,
                                    WA_Activate,        TRUE,
                                    WA_RMBTrap,         TRUE,
                                    WA_SimpleRefresh,   TRUE,
                                    WA_BackFill,        LAYERS_NOBACKFILL,
                                    WA_IDCMP,           IDCMP_RAWKEY|IDCMP_GADGETUP|IDCMP_GADGETDOWN|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
                                    WA_ReportMouse,     TRUE,
                                    TAG_DONE))
                                {
                                    CINIT(ucl, COP_LEN);
                                    CWAIT(ucl, 0, 0);
                                    CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                                    CEND(ucl);

                                    AddIntServer(INTB_COPER, is);

                                    Forbid();
                                    s->ViewPort.UCopIns = ucl;
                                    Permit();

                                    RethinkDisplay();

                                    return(s);
                                }
                                FreeMem(ucl, sizeof(*ucl));
                            }
                            FreeSignal(sd->cop.signal);
                        }
                        DeleteMsgPort(sd->mp);
                    }
                    FreeScreenBuffer(s, sd->sb[1]);
                }
                FreeScreenBuffer(s, sd->sb[0]);
            }
            CloseScreen(s);
        }
        FreeMem(sd, sizeof(*sd));
    }
    return(NULL);
}

void handleScreenDraw(struct Screen *s)
{
    SD sd = (SD)s->UserData;

    if (!sd->safe)
    {
        while (!GetMsg(sd->mp))
        {
            WaitPort(sd->mp);
        }
        sd->safe = TRUE;
    }

    if (sd->draw)
    {
        sd->draw(s, sd->sb[sd->frame]->sb_BitMap, sd->frame);
    }
}

void handleScreenChange(struct Screen *s)
{
    SD sd = (SD)s->UserData;

    WaitBlit();
    while (!ChangeScreenBuffer(s, sd->sb[sd->frame]))
    {
        WaitTOF();
    }

    sd->frame ^= 1;
    sd->safe = FALSE;
}

void handleWindow(struct Screen *s)
{
    struct IntuiMessage *msg;
    SD sd = (SD)s->UserData;

    struct Window *w = sd->w;

    if (sd->handle)
    {
        if (msg = (struct IntuiMessage *)GetMsg(w->UserPort))
        {
            sd->handle(w, msg);
            ReplyMsg((struct Message *)msg);
        }
    }
}

ULONG obtainScreenSignal(struct Screen *s)
{
    SD sd = (SD)s->UserData;

    return((1L << sd->mp->mp_SigBit) | (1L << sd->cop.signal) | (1L << sd->w->UserPort->mp_SigBit));
}

struct Window *obtainWindow(struct Screen *s)
{
    SD sd = (SD)s->UserData;

    return(sd->w);
}

void handleScreen(struct Screen *s, ULONG sigmask)
{
    SD sd = (SD)s->UserData;

    if (sigmask & (1L << sd->mp->mp_SigBit))
    {
        handleScreenDraw(s);
    }

    if (sigmask & (1L << sd->cop.signal))
    {
        handleScreenChange(s);
    }

    if (sigmask & (1L << sd->w->UserPort->mp_SigBit))
    {
        handleWindow(s);
    }
}

void closeScreen(struct Screen *s)
{
    SD sd = (SD)s->UserData;

    CloseWindow(sd->w);

    RemIntServer(INTB_COPER, &sd->is);
    FreeSignal(sd->cop.signal);

    if (!sd->safe)
    {
        while (!GetMsg(sd->mp))
        {
            WaitPort(sd->mp);
        }
    }
    DeleteMsgPort(sd->mp);
    FreeScreenBuffer(s, sd->sb[1]);
    FreeScreenBuffer(s, sd->sb[0]);
    CloseScreen(s);
    FreeMem(sd, sizeof(*sd));
}
