
/* $Id$ */

#include <stdio.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <graphics/gfxmacros.h>
#include <dos/dos.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>

#define KICK    39 /* Minimum kickstart */
#define WIDTH   320 /* Screen mode dimensions */
#define HEIGHT  256
#define DEPTH   5
#define COP_PRI 0 /* Copper interrupt priority */
#define COP_LEN 3 /* Length of copperlist */

extern struct Library *SysBase;

struct Library *IntuitionBase, *GfxBase, *IFFParseBase, *DiskfontBase;

extern void myCopper(void);
far extern struct Custom custom;

enum
{
    ERR_REQ,
    ERR_MODEID,
    ERR_FOUND_MODEID,
    ERRORS
};

enum
{
    SIG_SAFE,
    SIG_COPPER,
    SIG_USER,
    SIGNALS
};

struct screenInfo
{
    struct BitMap *bm[2];
    struct ScreenBuffer *sb[2];
    struct MsgPort *safemp;
    BOOL safe;
    WORD frame;
    struct Interrupt is;
    struct copperInfo
    {
        struct ViewPort *vp;
        WORD signal;
        struct Task *task;
    } cop;
};

STRPTR error[ERRORS] =
{
    "This program requires %s (V%ld)!",
    "Can't get display ID for %d x %d x %d (MonitorID 0x%lx)!",
    "Found displayID: 0x%lx (%s)"
};

struct EasyStruct es =
{
    sizeof(struct EasyStruct),
    0,
    "Game message",
    NULL,
    "Quit"
};

BOOL openLibs(void)
{
    es.es_TextFormat = error[ERR_REQ];

    if (SysBase->lib_Version < KICK)
    {
        if (SysBase->lib_Version < 36)
        {
            /* Put just error string on earlier intuition */
            puts(error[0]);
        }
        else
        {
            /* Show requester to the user */
            EasyRequest(NULL, &es, NULL, "kickstart 3.0", KICK);
        }
        return(FALSE);
    }

    if (!(IntuitionBase = OpenLibrary("intuition.library", KICK)))
    {
        EasyRequest(NULL, &es, NULL, "intuition.library", KICK);
        return(FALSE);
    }
    else
    {
        if (!(GfxBase = OpenLibrary("graphics.library", KICK)))
        {
            EasyRequest(NULL, &es, NULL, "graphics.library", KICK);
        }
        else
        {
            if (!(IFFParseBase = OpenLibrary("iffparse.library", KICK)))
            {
                EasyRequest(NULL, &es, NULL, "iffparse.library", KICK);
            }
            else
            {
                if (!(DiskfontBase = OpenLibrary("diskfont.library", KICK)))
                {
                    EasyRequest(NULL, &es, NULL, "diskfont.library", KICK);
                }
                else
                {
                    return(TRUE);
                }
                CloseLibrary(IFFParseBase);
            }
            CloseLibrary(GfxBase);
        }
        CloseLibrary(IntuitionBase);
    }
    return(FALSE);
}

void closeLibs(void)
{
    CloseLibrary(DiskfontBase);
    CloseLibrary(IFFParseBase);
    CloseLibrary(GfxBase);
    CloseLibrary(IntuitionBase);
}

ULONG getMonitorID(void)
{
    struct Screen *pubs;

    if (pubs = LockPubScreen(NULL))
    {
        ULONG monID = GetVPModeID(&pubs->ViewPort) & MONITOR_ID_MASK;
        if (monID == INVALID_ID)
        {
            printf("Couldn't get monitor ID!\n");
        }

        UnlockPubScreen(NULL, pubs);
        return(monID);
    }
    else
        printf("Couldn't lock public screen!\n");
    return(INVALID_ID);
}

ULONG getModeID(UWORD width, UWORD height, UBYTE depth)
{
    ULONG monID;
    if ((monID = getMonitorID()) != INVALID_ID)
    {
        ULONG modeID = BestModeID(
            BIDTAG_MonitorID,       monID,
            BIDTAG_NominalWidth,    width,
            BIDTAG_NominalHeight,   height,
            BIDTAG_Depth,           depth,
            TAG_DONE);

        if (modeID == INVALID_ID)
        {
            es.es_TextFormat = error[ERR_MODEID];
            EasyRequest(NULL, &es, NULL, width, height, depth, monID);
        }

        return(modeID);
    }
    return(INVALID_ID);
}

struct Screen *openScreen(ULONG modeID, UBYTE depth, struct Rectangle *dclip)
{
    UWORD width = dclip->MaxX - dclip->MinX + 1;
    UWORD height = dclip->MaxY - dclip->MinY + 1;
    struct BitMap *bm[2];
    struct Screen *s;

    if (bm[0] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE|BMF_CLEAR, NULL))
    {
        if (bm[1] = AllocBitMap(width, height, depth, BMF_DISPLAYABLE, NULL))
        {
            if (s = OpenScreenTags(NULL,
                SA_DClip,       dclip,
                SA_BitMap,      bm[0],
                SA_DisplayID,   modeID,
                SA_Quiet,       TRUE,
                SA_Exclusive,   TRUE,
                SA_ShowTitle,   FALSE,
                SA_BackFill,    LAYERS_NOBACKFILL,
                TAG_DONE))
            {
                struct screenInfo *si;

                if (si = AllocMem(sizeof(*si), MEMF_PUBLIC|MEMF_CLEAR))
                {
                    si->bm[0] = bm[0];
                    si->bm[1] = bm[1];
                    if (si->safemp = CreateMsgPort())
                    {
                        if (si->sb[0] = AllocScreenBuffer(s, bm[0], 0))
                        {
                            if (si->sb[1] = AllocScreenBuffer(s, bm[1], 0))
                            {
                                si->sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = si->safemp;
                                si->sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = si->safemp;
                                si->safe = TRUE;
                                si->frame = 1;
                                s->UserData = (APTR)si;
                                return(s);
                            }
                            FreeScreenBuffer(s, si->sb[0]);
                        }
                        DeleteMsgPort(si->safemp);
                    }
                    FreeMem(si, sizeof(*si));
                }
                CloseScreen(s);
            }
            FreeBitMap(bm[1]);
        }
        FreeBitMap(bm[0]);
    }
    return(NULL);
}

void closeScreen(struct Screen *s)
{
    struct screenInfo *si = (struct screenInfo *)s->UserData;

    if (!si->safe)
    {
        while (!GetMsg(si->safemp))
        {
            WaitPort(si->safemp);
        }
    }
    FreeScreenBuffer(s, si->sb[1]);
    FreeScreenBuffer(s, si->sb[0]);
    DeleteMsgPort(si->safemp);
    CloseScreen(s);
    FreeBitMap(si->bm[1]);
    FreeBitMap(si->bm[0]);
    FreeMem(si, sizeof(*si));
}

BOOL addCopper(struct Screen *s)
{
    struct screenInfo *si = (struct screenInfo *)s->UserData;
    struct Interrupt *is = &si->is;
    struct copperInfo *ci = &si->cop;

    is->is_Code = myCopper;
    is->is_Data = (APTR)ci;
    is->is_Node.ln_Pri = COP_PRI;
    is->is_Node.ln_Name = "Magazyn";

    ci->vp = &s->ViewPort;
    ci->task = FindTask(NULL);
    if ((ci->signal = AllocSignal(-1)) != -1)
    {
        struct UCopList *ucl;

        if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
        {
            CINIT(ucl, COP_LEN);
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
        FreeSignal(ci->signal);
    }
    return(FALSE);
}

void remCopper(struct Screen *s)
{
    struct screenInfo *si = (struct screenInfo *)s->UserData;

    RemIntServer(INTB_COPER, &si->is);
    FreeSignal(si->cop.signal);
}

struct Screen *init(void)
{
    struct NameInfo ni;
    struct DimensionInfo dims;

    if (openLibs())
    {
        ULONG modeID;
        if ((modeID = getModeID(WIDTH, HEIGHT, DEPTH)) != INVALID_ID)
        {
            if (GetDisplayInfoData(NULL, (UBYTE *)&ni, sizeof(ni), DTAG_NAME, modeID) > 0)
            {
                es.es_TextFormat = error[ERR_FOUND_MODEID];
                es.es_GadgetFormat = "Ok|Change";
                EasyRequest(NULL, &es, NULL, modeID, ni.Name);

                /* Obtain bitmap dimensions */
                if (GetDisplayInfoData(NULL, (UBYTE *)&dims, sizeof(dims), DTAG_DIMS, modeID) > 0)
                {
                    struct Screen *s;
                    if (s = openScreen(modeID, DEPTH, &dims.Nominal))
                    {
                        if (addCopper(s))
                        {
                            return(s);
                        }
                        closeScreen(s);
                    }
                }
            }
        }
        closeLibs();
    }
    return(NULL);
}

void play(struct Screen *s)
{
    struct screenInfo *si = (struct screenInfo *)s->UserData;
    ULONG signals[SIGNALS] = { 0 }, total = 0L;
    BOOL done = FALSE;
    WORD counter = 0;

    signals[SIG_SAFE] = 1L << si->safemp->mp_SigBit;
    signals[SIG_COPPER] = 1L << si->cop.signal;

    total = signals[SIG_SAFE] | signals[SIG_COPPER];

    while (!done)
    {

        ULONG result = Wait(total);
        if (result & signals[SIG_SAFE])
        {
            if (!si->safe)
            {
                while (!GetMsg(si->safemp))
                {
                    WaitPort(si->safemp);
                }
                si->safe = TRUE;
            }
        }

        if ((result & signals[SIG_COPPER]) && si->safe)
        {
            counter++;
            if (counter == 200)
            {
                done = TRUE;
            }
        }
    }
}

void cleanup(struct Screen *s)
{
    remCopper(s);
    closeScreen(s);
    closeLibs();
}

int main(void)
{
    struct Screen *s;

    if (s = init())
    {
        play(s);
        cleanup(s);
    }
    return(RETURN_OK);
}
