
/* Intuition functions */

#include <stdio.h>
#include <intuition/intuition.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <graphics/gfxmacros.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/exec_protos.h>

enum
{
    SAFE,
    DISP
};

enum
{
    S_USER,
    S_SAFE,
    S_DISP,
    SIGS
};

struct copperData
{
    struct ViewPort *vp;
    WORD signal;
    struct Task *task;
};

struct screenData
{
    struct Screen *s;
    struct ScreenBuffer *sb[2];
    struct MsgPort *safeport;
    BOOL safe[2];
    UWORD frame;
    union
    {
        struct MsgPort *port;
        struct Interrupt *is;
    } disp;
    ULONG signal;
};

extern __far struct Custom custom;
extern void myCopper();

BOOL useCopper = TRUE; /* Use copper or display message for sync? */
BOOL useLores = TRUE;

BOOL addSync(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;

    if (!useCopper)
    {
        if (sd->disp.port = CreateMsgPort())
        {
            sd->sb[0]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = sd->disp.port;
            sd->sb[1]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = sd->disp.port;

            sd->signal = 1L << sd->disp.port->mp_SigBit;
            return(TRUE);
        }
    }
    else
    {
        struct Interrupt *is;
        if (sd->disp.is = is = AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC))
        {
            struct copperData *cd;

            if (cd = AllocMem(sizeof(*cd), MEMF_PUBLIC))
            {
                struct UCopList *ucl;

                is->is_Data = (APTR)cd;

                cd->vp = &s->ViewPort;
                cd->task = FindTask(NULL);

                is->is_Code = myCopper;
                is->is_Node.ln_Pri = 0;
                is->is_Node.ln_Name = "Magazyn";

                if ((cd->signal = AllocSignal(-1)) != -1)
                {
                    if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                    {
                        CINIT(ucl, 3);
                        CWAIT(ucl, 0, 0);
                        CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                        CEND(ucl);

                        Forbid();
                        s->ViewPort.UCopIns = ucl;
                        Permit();

                        RethinkDisplay();

                        AddIntServer(INTB_COPER, is);
                        return(TRUE);
                    }
                    FreeSignal(cd->signal);
                }
                FreeMem(cd, sizeof(*cd));
            }
            FreeMem(is, sizeof(*is));
        }
    }
    return(FALSE);
}

void remSync(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;

    if (!useCopper)
    {
        if (!sd->safe[DISP])
        {
            while (!GetMsg(sd->disp.port))
            {
                WaitPort(sd->disp.port);
            }
        }
        DeleteMsgPort(sd->disp.port);
    }
    else
    {
        struct copperData *cd = (struct copperData *)sd->disp.is->is_Data;

        RemIntServer(INTB_COPER, sd->disp.is);
        FreeSignal(cd->signal);
        FreeMem(cd, sizeof(*cd));
        FreeMem(sd->disp.is, sizeof(*sd->disp.is));
    }
}

BOOL addDBuf(struct Screen *s)
{
    struct screenData *sd;

    if (sd = AllocMem(sizeof(*sd), MEMF_PUBLIC))
    {
        s->UserData = (APTR)sd;
        sd->s = s;
        if (sd->sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
        {
            if (sd->sb[1] = AllocScreenBuffer(s, NULL, SB_COPY_BITMAP))
            {
                if (sd->safeport = CreateMsgPort())
                {
                    sd->safe[SAFE] = sd->safe[DISP] = TRUE;
                    sd->frame = 1;

                    sd->sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sd->safeport;
                    sd->sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sd->safeport;

                    /* Add copper or display sync */
                    if (addSync(s))
                    {
                        return(TRUE);
                    }
                    DeleteMsgPort(sd->safeport);
                }
                FreeScreenBuffer(s, sd->sb[1]);
            }
            FreeScreenBuffer(s, sd->sb[0]);
        }
        FreeMem(sd, sizeof(*sd));
    }
    return(FALSE);
}

void remDBuf(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;

    remSync(s);

    if (!sd->safe[SAFE])
    {
        while (!GetMsg(sd->safeport))
        {
            WaitPort(sd->safeport);
        }
    }
    DeleteMsgPort(sd->safeport);
    FreeScreenBuffer(s, sd->sb[1]);
    FreeScreenBuffer(s, sd->sb[0]);
    FreeMem(sd, sizeof(*sd));
}

/* Open screen at given depth */

struct Screen *openScreen(UBYTE depth, struct TextAttr *ta)
{
    struct Screen *s;
    struct Rectangle dclip = { 0, 0, 319, 255 };
    ULONG modeID = LORES_KEY;

    if (s = OpenScreenTags(NULL,
        SA_Depth,           depth,
        SA_DClip,           &dclip,
        SA_DisplayID,       modeID,
        SA_LikeWorkbench,   TRUE,
        SA_Font,            ta,
        SA_Title,           "Magazyn",
        SA_Interleaved,     FALSE,
        SA_ShowTitle,       FALSE,
        SA_Quiet,           TRUE,
        SA_Exclusive,       TRUE,
        TAG_DONE))
    {
        /* Add double-buffering and copper/sync stuff */
        if (addDBuf(s))
        {
            return(s);
        }
        CloseScreen(s);
    }
    return(NULL);
}

void closeScreen(struct Screen *s)
{
    remDBuf(s);
    CloseScreen(s);
}

/* Open backdrop window */

struct Window *openWindow(struct Screen *s)
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
        WA_RMBTrap,         TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_ReportMouse,     TRUE,
        WA_IDCMP,           IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE,
        TAG_DONE))
    {
        return(w);
    }
    return(NULL);
}

BOOL handleUser(struct Window *w)
{
    struct IntuiMessage *msg;

    while (msg = (struct IntuiMessage *)GetMsg(w->UserPort))
    {
        if (msg->Class == IDCMP_RAWKEY)
        {
            if (msg->Code == 0x45)
            {
                return(TRUE);
            }
        }
        ReplyMsg((struct Message *)msg);
    }
    return(FALSE);
}

void loop(struct Window *w)
{
    struct Screen *s = w->WScreen;
    struct screenData *sd = (struct screenData *)s->UserData;
    ULONG signals[SIGS], total;
    BOOL done = FALSE;
    WORD counter = 0;
    UWORD frame = sd->frame;

    signals[S_USER] = 1L << w->UserPort->mp_SigBit;
    signals[S_SAFE] = 1L << sd->safeport->mp_SigBit;

    if (!useCopper)
    {
        signals[S_DISP] = 1L << sd->disp.port->mp_SigBit;

        if (!ChangeScreenBuffer(s, sd->sb[frame]))
        {
            WaitTOF();
        }
        sd->frame = frame ^= 1;
        sd->safe[SAFE] = sd->safe[DISP] = FALSE;
    }
    else
    {
        signals[S_DISP] = 1L << ((struct copperData *)sd->disp.is->is_Data)->signal;
    }

    total = signals[0] | signals[1] | signals[2];

    while (!done)
    {
        ULONG result = Wait(total);

        if (result & signals[S_USER])
        {
            done = handleUser(w);
        }

        if (result & signals[S_SAFE])
        {
            if (!sd->safe[SAFE])
            {
                while (!GetMsg(sd->safeport))
                {
                    WaitPort(sd->safeport);
                }
                sd->safe[SAFE] = TRUE;
            }
        }

        if (result & signals[S_DISP])
        {
            if (!useCopper && !sd->safe[DISP])
            {
                while (!GetMsg(sd->disp.port))
                {
                    WaitPort(sd->disp.port);
                }
                sd->safe[DISP] = TRUE;
            }

            if (sd->safe[SAFE])
            {
                sd->safe[DISP] = TRUE;
                while (!ChangeScreenBuffer(s, sd->sb[frame]))
                {
                    WaitTOF();
                }
                sd->frame = frame ^= 1;
                sd->safe[SAFE] = sd->safe[DISP] = FALSE;
                if (++counter == 200)
                {
                    done = TRUE;
                }
            }
        }
    }
    printf("%d\n", counter);
}

int main(void)
{
    struct TextAttr ta = { "ld.font", 8, FS_NORMAL, FPF_DISKFONT | FPF_DESIGNED };
    struct TextFont *tf;
    struct Screen *s;
    struct Window *w;

    if (tf = OpenDiskFont(&ta))
    {
        if (s = openScreen(5, &ta))
        {
            if (w = openWindow(s))
            {
                loop(w);
                CloseWindow(w);
            }
            closeScreen(s);
        }
        CloseFont(tf);
    }
    return(0);
}
