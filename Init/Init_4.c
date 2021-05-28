
/*
 * $Log$
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <intuition/screens.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <graphics/gfxmacros.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/layers_protos.h>

#include "Init.h"
#include "Game.h"

#define OPTIONS  5

#define UP_KEY    0x4C
#define DOWN_KEY  0x4D
#define RIGHT_KEY 0x4E
#define LEFT_KEY  0x4F

#define F9_KEY    0x58
#define F10_KEY   0x59

#define ESC_KEY 0x45

#define DEPTH    5
#define COMMANDS 3 /* Copper-list commands */
#define COP_PRI  0

far extern struct Custom custom;
extern void myCopper(void);

enum
{
    SIGNAL_IDCMP,
    SIGNAL_REQ,
    SIGNAL_SAFE,
    SIGNAL_COPPER,
    SIGNALS
};

struct screenData
{
    struct Screen *s;
    struct ScreenBuffer *sb[2];
    struct MsgPort *safemp;
    BOOL safe;
    UWORD frame;
    struct Interrupt is;
    struct copperData
    {
        struct ViewPort *vp;
        WORD signal;
        struct Task *task;
    } cd;
    struct BitMap *gfx;
    struct boardData bd;
};

struct windowData
{
    struct Requester req;
    struct Window *w;
    ULONG (*handleIDCMP)(struct IntuiMessage *msg);
    ULONG (*animate)(struct windowData *wd, UWORD frame, struct BitMap *bm);
    BOOL drawn[2];
    struct Region *damage;
    BOOL init;
    WORD active;
};

struct reqData
{
    struct windowData wd;
    WORD active, prevActive;
};

STRPTR errors[] =
{
    "No errors",
    "Couldn't open intuition.library V39!",
    "Couldn't open iffparse.library V39!",
    "Couldn't open diskfont.library V39!",
    "Couldn't open screen!",
    "Out of memory!",
    "Out of graphics memory!",
    "Couldn't create message port!",
    "Couldn't alloc signal!",
    "Couldn't open window!",
    "Couldn't open file!",
    "Escape pressed",
    "Close pressed"
};

struct Library *IntuitionBase, *IFFParseBase, *DiskfontBase;

VOID printError(ULONG err)
{
    assert(err < ERROR_CODES);
    puts(errors[err]);
}

ULONG openLibs(VOID)
{
    ULONG err;

    if (!(IntuitionBase = OpenLibrary("intuition.library", 39L)))
        err = NO_INTUI;
    else
    {
        if (!(IFFParseBase = OpenLibrary("iffparse.library", 39L)))
            err = NO_IFFPARSE;
        else
        {
            if (!(DiskfontBase = OpenLibrary("diskfont.library", 39L)))
                err = NO_DISKFONT;
            else
                return(NO_ERRORS);
            CloseLibrary(IFFParseBase);
        }
        CloseLibrary(IntuitionBase);
    }
    return(err);
}

VOID closeLibs(VOID)
{
    CloseLibrary(DiskfontBase);
    CloseLibrary(IFFParseBase);
    CloseLibrary(IntuitionBase);
}

struct Screen *openScreen(ULONG *errPtr)
{
    struct Screen *s;
    ULONG err;
    struct Rectangle dclip =
    {
        0, 0, 319, 255
    };
    const UBYTE depth = DEPTH;
    const ULONG modeID = LORES_KEY;
    const WORD commands = COMMANDS;
    const BYTE pri = COP_PRI;

    if (!(s = OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_Depth,       depth,
        SA_DisplayID,   modeID,
        SA_Quiet,       TRUE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_ShowTitle,   FALSE,
        SA_Exclusive,   TRUE,
        SA_Draggable,   FALSE,
        SA_Title,       "Gear Works",
        TAG_DONE)))
        err = NO_SCREEN;
    else
    {
        struct screenData *sd;
        if (!(sd = AllocMem(sizeof(*sd), MEMF_PUBLIC|MEMF_CLEAR)))
            err = NO_MEM;
        else
        {
            s->UserData = (APTR)sd;
            sd->s = s;

            if (!(sd->sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP)))
                err = NO_GFXMEM;
            else
            {
                if (!(sd->sb[1] = AllocScreenBuffer(s, NULL, 0)))
                    err = NO_GFXMEM;
                else
                {
                    if (!(sd->safemp = CreateMsgPort()))
                        err = NO_MSGPORT;
                    else
                    {
                        struct UCopList *ucl;

                        sd->safe = TRUE;
                        sd->frame = 1;

                        sd->sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sd->safemp;
                        sd->sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sd->safemp;

                        if (!(ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR)))
                            err = NO_MEM;
                        else
                        {
                            struct Interrupt *is = &sd->is;
                            struct copperData *cd = &sd->cd;

                            CINIT(ucl, commands);
                            CWAIT(ucl, 0, 0);
                            CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                            CEND(ucl);

                            Forbid();
                            s->ViewPort.UCopIns = ucl;
                            Permit();

                            RethinkDisplay();

                            if ((cd->signal = AllocSignal(-1)) == -1)
                                err = NO_SIGNAL;
                            else
                            {
                                cd->vp = &s->ViewPort;
                                cd->task = FindTask(NULL);

                                is->is_Code = myCopper;
                                is->is_Data = (APTR)cd;
                                is->is_Node.ln_Pri = pri;
                                is->is_Node.ln_Name = "Gear Works";

                                AddIntServer(INTB_COPER, is);

                                return(s);
                            }
                        }
                        DeleteMsgPort(sd->safemp);
                    }
                    FreeScreenBuffer(s, sd->sb[1]);
                }
                FreeScreenBuffer(s, sd->sb[0]);
            }
            FreeMem(sd, sizeof(*sd));
        }
        CloseScreen(s);
    }
    if (errPtr)
        *errPtr = err;
    return(NULL);
}

VOID closeScreen(struct Screen *s)
{
    struct screenData *sd = (struct screenData *)s->UserData;

    RemIntServer(INTB_COPER, &sd->is);

    FreeSignal(sd->cd.signal);

    if (!sd->safe)
        while (!GetMsg(sd->safemp))
            WaitPort(sd->safemp);

    DeleteMsgPort(sd->safemp);
    FreeScreenBuffer(s, sd->sb[1]);
    FreeScreenBuffer(s, sd->sb[0]);
    FreeMem(sd, sizeof(*sd));
    CloseScreen(s);
}

ULONG handleBackdrop(struct IntuiMessage *msg)
{
    ULONG class = msg->Class;
    UWORD code = msg->Code;
    WORD mouseX = msg->MouseX, mouseY = msg->MouseY;
    APTR iaddr = msg->IAddress;
    struct Window *bdw = msg->IDCMPWindow;
    struct windowData *bdwd = (struct windowData *)bdw->UserData;
    ULONG err = NO_ERRORS;
    struct Rectangle r;

    switch (class)
    {
        case IDCMP_RAWKEY:
            switch (code)
            {
                case ESC_KEY:
                    err = RESULT_ESC;
                    break;

                case F9_KEY:
                    if (bdwd->active == 0)
                        bdwd->active = -1;
                    else
                        bdwd->active = 0;
                    r.MinX = 0;
                    r.MinY = 174;
                    r.MaxX = 159;
                    r.MaxY = 255;
                    bdwd->drawn[0] = bdwd->drawn[1] = FALSE;
                    OrRectRegion(bdwd->damage, &r);
                    break;

                case F10_KEY:
                    if (bdwd->active == 1)
                        bdwd->active = -1;
                    else
                        bdwd->active = 1;
                    r.MinX = 0;
                    r.MinY = 174;
                    r.MaxX = 159;
                    r.MaxY = 255;
                    bdwd->drawn[0] = bdwd->drawn[1] = FALSE;
                    OrRectRegion(bdwd->damage, &r);
                    break;
            }
            break;
        case IDCMP_MOUSEBUTTONS:
            break;
        case IDCMP_MOUSEMOVE:
            break;

        case IDCMP_REFRESHWINDOW:
            BeginRefresh(bdw);
            OrRegionRegion(bdw->WLayer->DamageList, bdwd->damage);
            bdwd->drawn[0] = bdwd->drawn[1] = FALSE;
            EndRefresh(bdw, TRUE);
            break;

    }
    return(err);
}

ULONG handleReq(struct IntuiMessage *msg)
{
    ULONG class = msg->Class;
    UWORD code = msg->Code;
    WORD mouseX = msg->MouseX, mouseY = msg->MouseY;
    APTR iaddr = msg->IAddress;
    ULONG err = NO_ERRORS;

    struct reqData *rd = (struct reqData *)msg->IDCMPWindow->UserData;

    switch (class)
    {
        case IDCMP_RAWKEY:
            switch (code)
            {
                case ESC_KEY:
                    err = RESULT_ESC;
                    break;

                case F10_KEY:
                    err = RESULT_CLOSE;
                    break;

                case DOWN_KEY:
                    if (rd->active < OPTIONS - 1)
                    {
                        rd->active++;
                        rd->wd.drawn[0] = rd->wd.drawn[1] = FALSE;
                    }
                    break;

                case UP_KEY:
                    if (rd->active > 0)
                    {
                        rd->active--;
                        rd->wd.drawn[0] = rd->wd.drawn[1] = FALSE;
                    }
                    break;
            }
            break;
        case IDCMP_MOUSEBUTTONS:
            break;

        case IDCMP_MOUSEMOVE:
            break;

    }

    return(err);
}

ULONG animateReq(struct windowData *wd, UWORD frame, struct BitMap *bm)
{
    struct RastPort *rp = wd->w->RPort;
    struct screenData *sd = (struct screenData *)wd->w->WScreen->UserData;
    WORD i;
    struct reqData *rd = (struct reqData *)wd;
    WORD active = rd->active;
    UBYTE *text[] =
    {
        "F1 Kontynuuj grë",
        "F2 Zapisz grë",
        "F3 Restartuj",
        "F4 Nowa gra",
        "F5 Wyjdú z gry"
    };
    rp->BitMap = bm;

    if (!wd->drawn[frame])
    {
        BltBitMapRastPort(sd->gfx, 0, 128, rp, 0, 0, 192, 128, 0xc0);

        /* Draw gadgets */

        if (wd->init)
        {
            SetDrMd(rp, JAM1);

            SetAPen(rp, 1);
            Move(rp, 84, 4 + rp->Font->tf_Baseline);
            Text(rp, "Magazyn v1.3", 12);
            SetAPen(rp, 10);
            Move(rp, 83, 3 + rp->Font->tf_Baseline);
            Text(rp, "Magazyn v1.3", 12);

            for (i = 0; i < OPTIONS; i++)
            {
                BltBitMapRastPort(sd->gfx, 0, 64, rp, 80, 28 + (i * 17), 80, 16, 0xc0);
                SetAPen(rp, 1);
/*                Move(rp, 84, 32 + (i * 17) + rp->Font->tf_Baseline);
                Text(rp, text[i], strlen(text[i]));*/
                SetAPen(rp, 10);
                Move(rp, 83, 31 + (i * 17) + rp->Font->tf_Baseline);
                Text(rp, text[i], strlen(text[i]));
            }
            if (wd->drawn[0] && wd->drawn[1])
                wd->init = FALSE;
        }

        if (rd->active != rd->prevActive)
        {
            if (rd->prevActive != -1)
            {
                SetAPen(rp, 7);

                Move(rp, 79, 27 + (rd->prevActive * 17));
                Draw(rp, 160, 27 + (rd->prevActive * 17));
                Draw(rp, 160, 44 + (rd->prevActive * 17));
                Draw(rp, 79, 44 + (rd->prevActive * 17));
                Draw(rp, 79, 28 + (rd->prevActive * 17));

                BltBitMapRastPort(sd->gfx, 0, 64, rp, 80, 28 + (rd->prevActive * 17), 80, 16, 0xc0);
                SetAPen(rp, 1);
/*                Move(rp, 84, 32 + (rd->prevActive * 17) + rp->Font->tf_Baseline);
                Text(rp, text[rd->prevActive], strlen(text[rd->prevActive]));*/
                SetAPen(rp, 10);
                Move(rp, 83, 31 + (rd->prevActive * 17) + rp->Font->tf_Baseline);
                Text(rp, text[rd->prevActive], strlen(text[rd->prevActive]));
            }

            SetAPen(rp, 12);

            Move(rp, 79, 27 + (active * 17));
            Draw(rp, 160, 27 + (active * 17));
            Draw(rp, 160, 44 + (active * 17));
            Draw(rp, 79, 44 + (active * 17));
            Draw(rp, 79, 28 + (active * 17));

            BltBitMapRastPort(sd->gfx, 0, 80, rp, 80, 28 + (active * 17), 80, 16, 0xc0);
            SetAPen(rp, 1);
/*            Move(rp, 84, 32 + (active * 17) + rp->Font->tf_Baseline);
            Text(rp, text[active], strlen(text[active])); */
            SetAPen(rp, 10);
            Move(rp, 83, 31 + (active * 17) + rp->Font->tf_Baseline);
            Text(rp, text[active], strlen(text[active]));
        }

        wd->drawn[frame] = TRUE;
    }
    return(NO_ERRORS);
}

ULONG animateBackdrop(struct windowData *wd, UWORD frame, struct BitMap *bm)
{
    struct RastPort *rp = wd->w->RPort;
    struct screenData *sd = (struct screenData *)wd->w->WScreen->UserData;
    struct boardData *bd = &sd->bd;
    struct Region *clip;
    WORD x, y;
    TILE *t = &bd->board[1][0];
    UBYTE *text[] =
    {
        "F9 Kafelek",
        "F10 Menu"
    };
    rp->BitMap = bm;

    if (!wd->drawn[frame])
    {
        WORD i;
        clip = InstallClipRegion(rp->Layer, wd->damage);

        BltBitMapRastPort(sd->gfx, 0, 96, rp, 0, 0, 320, 16, 0xc0);

        for (y = 1; y < BOARD_HEIGHT; y++)
        {
            for (x = 0; x < BOARD_WIDTH; x++)
            {
                BltBitMapRastPort(sd->gfx, t->subKind << 4, t->kind << 4, rp, x << 4, y << 4, 16, 16, 0xc0);
                t++;
            }
        }

        SetDrMd(rp, JAM1);

        for (i = 0; i < 2; i++)
        {
            WORD button = 64;
            if (i == wd->active)
                button = 80;

            BltBitMapRastPort(sd->gfx, 0, button, rp, (i * 80), 240, 80, 16, 0xc0);
            SetAPen(rp, 1);
/*            Move(rp, (80 * i) + 4, 244 + rp->Font->tf_Baseline);
            Text(rp, text[i], strlen(text[i])); */
            SetAPen(rp, 10);
            Move(rp, (80 * i) + 3, 243 + rp->Font->tf_Baseline);
            Text(rp, text[i], strlen(text[i]));
        }

        if (wd->active == 0)
        {
            SetAPen(rp, 31);
            Move(rp, 0, 175);
            Draw(rp, 48, 175);
            Draw(rp, 48, 239);

            BltBitMapRastPort(sd->gfx, 0, 0, rp, 0, 240 - 64, 48, 64, 0xc0);
        }

        wd->drawn[frame] = TRUE;

        InstallClipRegion(rp->Layer, clip);

        if (wd->drawn[0] && wd->drawn[1])
        {
            ClearRegion(wd->damage);
        }
    }
    return(NO_ERRORS);
}

struct Window *openWindow(struct Screen *s, ULONG *errPtr)
{
    struct Window *w;
    ULONG err;
    const ULONG idcmp = IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_REFRESHWINDOW;

    if (!(w = OpenWindowTags(NULL,
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
        WA_IDCMP,           idcmp,
        TAG_DONE)))
        err = NO_WINDOW;
    else
    {
        struct windowData *wd;
        if (!(wd = AllocMem(sizeof(*wd), MEMF_PUBLIC|MEMF_CLEAR)))
            err = NO_MEM;
        else
        {
            if (!(wd->damage = NewRegion()))
                err = NO_MEM;
            else
            {
                w->UserData = (APTR)wd;
                wd->w = w;
                wd->handleIDCMP = handleBackdrop;
                wd->animate = animateBackdrop;

                return(w);
            }
            FreeMem(wd, sizeof(*wd));
        }
        CloseWindow(w);
    }
    if (errPtr)
        *errPtr = err;
    return(NULL);
}

struct Window *openReq(struct Window *p, ULONG *errPtr)
{
    struct windowData *pd = (struct windowData *)p->UserData;
    struct Window *w;
    ULONG err;
    const ULONG idcmp = IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE|IDCMP_REFRESHWINDOW;

/*    Request(&pd->req, p); */

    if (!(w = OpenWindowTags(NULL,
        WA_CustomScreen,    p->WScreen,
        WA_Left,            64,
        WA_Top,             64,
        WA_Width,           192,
        WA_Height,          128,
        WA_Backdrop,        FALSE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_ReportMouse,     TRUE,
        WA_IDCMP,           idcmp,
        TAG_DONE)))
        err = NO_WINDOW;
    else
    {
        struct windowData *wd;
        if (!(wd = AllocMem(sizeof(struct reqData), MEMF_PUBLIC|MEMF_CLEAR)))
            err = NO_MEM;
        else
        {
            w->UserData = (APTR)wd;
            wd->w = w;
            wd->handleIDCMP = handleReq;
            wd->animate = animateReq;

            wd->init = TRUE;
            ((struct reqData *)wd)->prevActive = -1;
            ((struct reqData *)wd)->active = 0;

            return(w);
        }
        CloseWindow(w);
    }
    if (errPtr)
        *errPtr = err;

/*    EndRequest(&pd->req, p); */
    return(NULL);
}

VOID closeWindow(struct Window *w)
{
    struct windowData *wd = (struct windowData *)w->UserData;

    DisposeRegion(wd->damage);

    FreeMem(wd, sizeof(*wd));
    CloseWindow(w);
}

VOID closeReq(struct Window *w, struct Window *p)
{
    struct windowData *pd = (struct windowData *)p->UserData;
    struct windowData *wd = (struct windowData *)w->UserData;

/*    EndRequest(&pd->req, p); */

    FreeMem(wd, sizeof(struct reqData));
    CloseWindow(w);
}

ULONG initAll(struct initData *id)
{
    ULONG err = NO_ERRORS;
    struct screenData *sd;
    struct TextAttr ta = { "centurion.font", 9, FS_NORMAL, FPF_DISKFONT|FPF_DESIGNED };

    if ((err = openLibs()) == NO_ERRORS)
    {
        if (id->s = openScreen(&err))
        {
            sd = (struct screenData *)id->s->UserData;
            if (id->bdw = openWindow(id->s, &err))
            {
                ((struct windowData *)id->bdw->UserData)->active = 1;
                if (id->reqw = openReq(id->bdw, &err))
                {
                    if (loadBoard("Data1/Levels/Level001.iff", &sd->bd))
                    {
                        if (id->tf = OpenDiskFont(&ta))
                        {
                            struct Rectangle rect;
                            struct windowData *bdwd = (struct windowData *)id->bdw->UserData;
                            bdwd->drawn[0] = bdwd->drawn[1] = FALSE;

                            rect.MinX = rect.MinY = 0;
                            rect.MaxX = id->s->Width - 1;
                            rect.MaxY = id->s->Height - 1;
                            OrRectRegion(bdwd->damage, &rect);

                            SetFont(id->bdw->RPort, id->tf);
                            SetFont(id->reqw->RPort, id->tf);
                            SetFont(&id->s->RastPort, id->tf);

                            return(NO_ERRORS);
                        }
                        else
                            err = NO_FILE;
                    }
                    else
                        err = NO_FILE;
                    closeReq(id->reqw, id->bdw);
                }
                closeWindow(id->bdw);
            }
            closeScreen(id->s);
        }
        closeLibs();
    }
    return(err);
}

ULONG mainLoop(struct initData *id)
{
    struct screenData *sd = (struct screenData *)id->s->UserData;
    ULONG signals[SIGNALS], total;
    ULONG err = NO_ERRORS;

    total = signals[SIGNAL_IDCMP] = 1L << id->bdw->UserPort->mp_SigBit;
    total |= signals[SIGNAL_REQ] = 1L << id->reqw->UserPort->mp_SigBit;
    total |= signals[SIGNAL_SAFE] = 1L << sd->safemp->mp_SigBit;
    total |= signals[SIGNAL_COPPER] = 1L << sd->cd.signal;

    sd->gfx = id->gfx;

    while (err != RESULT_ESC)
    {
        ULONG result = Wait(total);

        if (result & signals[SIGNAL_IDCMP])
        {
            struct IntuiMessage *msg;

            while ((err == NO_ERRORS) && (msg = (struct IntuiMessage *)GetMsg(id->bdw->UserPort)))
            {
                struct windowData *bdwd = (struct windowData *)id->bdw->UserData;

                err = bdwd->handleIDCMP(msg);

                if (msg->Class == IDCMP_RAWKEY && msg->Code == F10_KEY)
                {
                    if (id->reqw = openReq(id->bdw, &err))
                    {
                        struct Rectangle r;

                        total |= signals[SIGNAL_REQ] = 1L << id->reqw->UserPort->mp_SigBit;

                        SetFont(id->reqw->RPort, id->tf);
                    }
                }
                ReplyMsg((struct Message *)msg);
            }
        }

        if (result & signals[SIGNAL_REQ])
        {
            struct IntuiMessage *msg;

            while ((err == NO_ERRORS) && (msg = (struct IntuiMessage *)GetMsg(id->reqw->UserPort)))
            {
                struct windowData *reqwd = (struct windowData *)id->reqw->UserData;

                err = reqwd->handleIDCMP(msg);
                ReplyMsg((struct Message *)msg);
            }

            if (err == RESULT_CLOSE)
            {
                struct windowData *bdwd = (struct windowData *)id->bdw->UserData;
                struct Rectangle r;

                closeReq(id->reqw, id->bdw);
                id->reqw = NULL;
                total &= ~signals[SIGNAL_REQ];
                signals[SIGNAL_REQ] = 0L;
                err = NO_ERRORS;
                bdwd->active = -1;
                r.MinX = 80;
                r.MinY = 240;
                r.MaxX = 159;
                r.MaxY = 255;
                bdwd->drawn[0] = bdwd->drawn[1] = FALSE;
                OrRectRegion(bdwd->damage, &r);
            }
        }

        if (result & signals[SIGNAL_SAFE])
        {
            struct windowData *bdwd = (struct windowData *)id->bdw->UserData;

            if (!sd->safe)
            {
                while (!GetMsg(sd->safemp))
                    WaitPort(sd->safemp);
                sd->safe = TRUE;
            }
            /* Safe to draw */

            if (id->reqw)
            {
                struct windowData *reqwd = (struct windowData *)id->reqw->UserData;
                if (reqwd->animate)
                {
                    reqwd->animate(reqwd, sd->frame, sd->sb[sd->frame]->sb_BitMap);
                }
            }

            if (bdwd->animate)
            {
                bdwd->animate(bdwd, sd->frame, sd->sb[sd->frame]->sb_BitMap);
            }
        }

        if (result & signals[SIGNAL_COPPER])
        {
            /* Safe to change */
            if (sd->safe)
            {
                WaitBlit();
                while (!ChangeScreenBuffer(sd->s, sd->sb[sd->frame]))
                    WaitTOF();

                sd->frame ^= 1;
                sd->safe = FALSE;
            }
        }
    }
    return(err);
}

VOID cleanAll(struct initData *id)
{
    CloseFont(id->tf);
    if (id->reqw)
        closeReq(id->reqw, id->bdw);
    closeWindow(id->bdw);
    closeScreen(id->s);
    closeLibs();
}
