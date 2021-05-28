
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <graphics/gfxmacros.h>
#include <exec/memory.h>
#include <exec/interrupts.h>

#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#define COPER_PRI     0
#define COPERLIST_LEN 3

#define ESC_KEY       0x45

#define DEPTH         5

#define MAPWIDTH      20
#define MAPHEIGHT     16

enum
{
    USER_SIG,
    COPER_SIG,
    SIGNALS
};

struct TextAttr ta =
{
    "topaz.font",
    8,
    FS_NORMAL,
    FPF_ROMFONT|FPF_DESIGNED
};

__far extern struct Custom custom;

extern void myCopper();

struct copperData
{
    UWORD signal;
    struct Task *task;
};

/* Open screen 1.3 compatible way */

struct Screen *openScreen()
{
    struct NewScreen ns;

    ns.LeftEdge = ns.TopEdge = 0;
    ns.Width = 320;
    ns.Height = 256;
    ns.Depth = DEPTH;
    ns.DetailPen = 1;
    ns.BlockPen = 0;
    ns.ViewModes = 0;
    ns.Type = CUSTOMSCREEN|SCREENQUIET;
    ns.Font = &ta;
    ns.DefaultTitle = "Magazyn";
    ns.Gadgets = NULL;
    ns.CustomBitMap = NULL;

    return(OpenScreen(&ns));
}

/* Open window */

struct Window *openWindow(struct Screen *s)
{
    struct NewWindow nw;

    nw.LeftEdge = nw.TopEdge = 0;
    nw.Width = 320;
    nw.Height = 256;
    nw.IDCMPFlags = IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE;
    nw.Flags = WFLG_BACKDROP|WFLG_BORDERLESS|WFLG_ACTIVATE|WFLG_SIMPLE_REFRESH|WFLG_NOCAREREFRESH|WFLG_RMBTRAP;
    nw.FirstGadget = NULL;
    nw.CheckMark = NULL;
    nw.Title = NULL;
    nw.Screen = s;
    nw.BitMap = NULL;
    nw.MinWidth = nw.MinHeight = nw.MaxWidth = nw.MaxHeight = 0;
    nw.Type = CUSTOMSCREEN;

    return(OpenWindow(&nw));
}

/* Add copper server */

BOOL addCopper(struct Screen *s, struct Interrupt *is, struct copperData *data)
{
    if ((data->signal = AllocSignal(-1)) != -1)
    {
        struct UCopList *ucl;

        data->task = FindTask(NULL);
        is->is_Code = myCopper;
        is->is_Data = (APTR)data;
        is->is_Node.ln_Pri = COPER_PRI;
        is->is_Node.ln_Name = "Magazyn";

        if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
        {
            CINIT(ucl, COPERLIST_LEN);
            CWAIT(ucl, 16, 0);
            CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
            CEND(ucl);

            Forbid();
            s->ViewPort.UCopIns = ucl;
            Permit();

            RethinkDisplay();

            AddIntServer(INTB_COPER, is);
            return(TRUE);
        }
        FreeSignal(data->signal);
    }
    return(FALSE);
}

void remCopper(struct Interrupt *is)
{
    RemIntServer(INTB_COPER, is);
    FreeSignal(((struct copperData *)is->is_Data)->signal);
}

void freeBitMap(struct BitMap *bm, UBYTE p)
{
    while (p > 0)
    {
        FreeRaster(bm->Planes[--p], 320, 256);
    }
}

/* Alloc bitmap 1.3 compatible way */

BOOL allocBitMap(struct BitMap *bm)
{
    WORD i;

    InitBitMap(bm, DEPTH, 320, 256);

    for (i = 0; i < DEPTH; i++)
    {
        if (!(bm->Planes[i] = AllocRaster(320, 256)))
        {
            freeBitMap(bm, i);
            return(FALSE);
        }
    }
    return(TRUE);
}

BOOL readBitMap(struct BitMap *bm, STRPTR name, struct ViewPort *vp)
{
    BPTR f;
    UWORD pal[32];
    if (f = Open(name, MODE_OLDFILE))
    {
        WORD i;
        for (i = 0; i < DEPTH; i++)
        {
            if (Read(f, bm->Planes[i], RASSIZE(320, 256)) != RASSIZE(320, 256))
            {
                Close(f);
                return(FALSE);
            }
        }
        if (Read(f, pal, sizeof(pal)) == sizeof(pal))
        {
            LoadRGB4(vp, pal, 32);
            Close(f);
            return(TRUE);
        }
        Close(f);
    }
    return(FALSE);
}

void updateRect(struct BitMap *bm, struct BitMap *dest, UWORD left, UWORD top, UWORD width, UWORD height, UBYTE mask)
{
    struct Custom *cust = &custom;
    WORD p;
    UWORD bpr = bm->BytesPerRow;
    LONG offset = (top * bpr) + (((left + 15) >> 4) << 1);
    WORD mod;

    width = (width + 15) >> 4;
    mod = bpr - (width << 1);

    OwnBlitter();

    for (p = 0; p < DEPTH; p++)
    {
        if (mask & (1 << p))
        {
            WaitBlit();

            cust->bltcon0 = SRCA | DEST | A_TO_D;
            cust->bltcon1 = 0;
            cust->bltapt  = bm->Planes[p] + offset;
            cust->bltdpt  = dest->Planes[p] + offset;
            cust->bltamod = mod;
            cust->bltdmod = mod;
            cust->bltafwm = 0xffff;
            cust->bltalwm = 0xffff;
            cust->bltsize = (height << 6) | width;
        }
    }

    DisownBlitter();
}

int play(struct Window *w, struct copperData *cop, struct BitMap *bm, struct BitMap *gfx)
{
    ULONG signals[SIGNALS] =
    {
        1L << w->UserPort->mp_SigBit,
        1L << cop->signal
    }, total = signals[USER_SIG] | signals[COPER_SIG];
    BOOL done = FALSE;
    struct RastPort *rp = w->RPort;
    struct BitMap *dest = w->RPort->BitMap;
    WORD x, y, tile;

    rp->BitMap = bm;

    BltBitMap(gfx, 0, 0, bm, 0, 0, 320, 16, 0xc0, 0xff, NULL);
    for (y = 1; y < MAPHEIGHT; y++)
    {
        for (x = 0; x < MAPWIDTH; x++)
        {
            if (x == 0 || x == MAPWIDTH - 1 || y == 0 || y == MAPHEIGHT - 1)
                tile = 1;
            else
                tile = 0;
            BltBitMap(gfx, tile << 4, 32, bm, x << 4, y << 4, 16, 16, 0xc0, 0xff, NULL);
        }
    }
    BltBitMap(gfx, 160, 48 + 0, bm, 31, 16 + 0, 112, 4, 0xc0, 0xff, NULL);

    SetSignal(0L, total);

    while (!done)
    {
        ULONG result = Wait(total);
        if (result & signals[USER_SIG])
        {
            struct IntuiMessage *msg;
            while (msg = (struct IntuiMessage *)GetMsg(w->UserPort))
            {
                if (msg->Class == IDCMP_RAWKEY)
                {
                    if (msg->Code == ESC_KEY)
                    {
                        done = TRUE;
                    }
                }
                ReplyMsg((struct Message *)msg);
            }
        }
        if (result & signals[COPER_SIG])
        {
            static WORD counter = 0;
            if (counter == 0)
            {
                WORD i;
                for (i = 0; i < 16; i++)
                {
                    updateRect(bm, dest, 0, i << 4, 320, 16, 0x1f);
                }
            }
            else if (counter < 32)
            {
                BltBitMap(gfx, 160, 48 + (counter << 2), bm, 31, 16 + (counter << 2), 112, 4, 0xc0, 0xff, NULL);
                updateRect(bm, dest, 32, 16 + (counter << 2), 112, 4, 0x1f);
            }
            else if (counter > 256)
            {
                done = TRUE;
            }
            counter++;
        }
    }
    return(0);
}

int main()
{
    struct Screen *s;

    if (s = openScreen())
    {
        struct Window *w;
        ShowTitle(s, FALSE);
        if (w = openWindow(s))
        {
            struct Interrupt is;
            struct copperData cop;
            struct BitMap bm, gfx;

            if (addCopper(s, &is, &cop))
            {
                if (allocBitMap(&bm))
                {
                    if (allocBitMap(&gfx))
                    {
                        if (readBitMap(&gfx, "Grafika1.raw", &s->ViewPort))
                        {
                            play(w, &cop, &bm, &gfx);
                        }
                        freeBitMap(&gfx, DEPTH);
                    }
                    freeBitMap(&bm, DEPTH);
                }
                remCopper(&is);
            }
            CloseWindow(w);
        }
        CloseScreen(s);
    }
    return(0);
}
