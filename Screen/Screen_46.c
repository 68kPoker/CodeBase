
/* $Header: Work:Magazyn/RCS/Screen.c,v 1.1 12/.0/.1 .2:.4:.3 Robert Exp $ */

/*===============================================*/

#include <stdio.h>
#include "debug.h"

#include <graphics/gfx.h>
#include <graphics/text.h>
#include <exec/memory.h>
#include <intuition/screens.h>
#include <hardware/custom.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <graphics/gfxmacros.h>

#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>

#include "Screen.h"

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

/*===============================================*/

BOOL initBitMaps(struct BitMap *bitmaps[], UWORD rasWidth, UWORD rasHeight, UBYTE rasDepth)
{
    D(bug("+ Allocating %d x %d x %d BitMaps.\n", rasWidth, rasHeight, rasDepth));
    if (bitmaps[0] = AllocBitMap(
        rasWidth,
        rasHeight,
        rasDepth,
        BMF_DISPLAYABLE | BMF_INTERLEAVED,
        NULL))
    {
        if (bitmaps[1] = AllocBitMap(
            rasWidth,
            rasHeight,
            rasDepth,
            BMF_DISPLAYABLE | BMF_INTERLEAVED,
            NULL))
        {
            return(TRUE);
        }
        FreeBitMap(bitmaps[0]);
    }
    D(bug("Error: Out of graphics memory!\n"));
    return(FALSE);
}

VOID freeBitMaps(struct BitMap *bitmaps[])
{
    D(bug("- Freeing bitmaps.\n"));
    FreeBitMap(bitmaps[1]);
    FreeBitMap(bitmaps[0]);
}

/*===============================================*/

BOOL initFont(struct TextFont **font, struct TextAttr *ta)
{
    D(bug("+ Opening %s size %d.\n", ta->ta_Name, ta->ta_YSize));
    if (*font = OpenDiskFont(ta))
    {
        return(TRUE);
    }
    D(bug("Error: Couldn't open %s size %d!\n", ta->ta_Name, ta->ta_YSize));
    return(FALSE);
}

VOID closeFont(struct TextFont *font)
{
    D(bug("- Closing font.\n"));
    CloseFont(font);
}

/*===============================================*/

ULONG *initPal(UBYTE depth)
{
    ULONG colors = 1L << depth;
    ULONG *pal;

    D(bug("+ Allocating palette.\n"));
    if (pal = AllocVec(((colors * 3) + 2) * sizeof(ULONG), MEMF_PUBLIC | MEMF_CLEAR))
    {
        pal[0] = colors << 16;

        pal[1 + (3 * 2)] = RGB(255);
        pal[2 + (3 * 2)] = RGB(255);
        pal[3 + (3 * 2)] = RGB(255);

        return(pal);
    }
    D(bug("Error: Out of memory!\n"));
    return(NULL);
}

VOID freePal(ULONG *pal)
{
    D(bug("- Freeing palette.\n"));
    FreeVec(pal);
}

/*===============================================*/

struct Screen *openScreen(STRPTR title, struct BitMap *bitmaps[], struct TextAttr *ta, ULONG *pal)
{
    ULONG modeID = LORES_KEY;
    struct Rectangle clip = { 0, 0, 15, 255 };
    struct Screen *s;

    D(bug("+ Opening screen.\n"));
    if (s = OpenScreenTags(NULL,
        SA_DClip,       &clip,
        SA_DisplayID,   modeID,
        SA_Title,       title,
        SA_BitMap,      bitmaps[0],
        SA_Font,        ta,
        TAG_IGNORE,     pal,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        return(s);
    }
    D(bug("Error: Couldn't open screen!\n"));
    return(NULL);
}

VOID closeScreen(struct Screen *s)
{
    D(bug("- Closing screen.\n"));
    CloseScreen(s);
}

/*===============================================*/

struct DBufInfo *addDBuf(struct ViewPort *vp, BOOL *safe, UWORD *frame)
{
    struct DBufInfo *dbi;

    D(bug("+ Allocating DBufInfo.\n"));
    if (dbi = AllocDBufInfo(vp))
    {
        if (dbi->dbi_SafeMessage.mn_ReplyPort = CreateMsgPort())
        {
            *safe = TRUE;
            *frame = 1;
            return(dbi);
        }
        else
            D(bug("Error: Couldn't create MsgPort!\n"));
        FreeDBufInfo(dbi);
    }
    else
        D(bug("Error: Couldn't alloc DBufInfo!\n"));
    return(NULL);
}

VOID remDBuf(struct DBufInfo *dbi, BOOL safe)
{
    struct MsgPort *mp = dbi->dbi_SafeMessage.mn_ReplyPort;

    D(bug("- Freeing DBufInfo.\n"));

    if (!safe)
    {
        while (!GetMsg(mp))
        {
            WaitPort(mp);
        }
    }
    DeleteMsgPort(mp);
    FreeDBufInfo(dbi);
}

/*===============================================*/

BOOL addUCopList(struct ViewPort *vp)
{
    struct UCopList *ucl;
    __far extern struct Custom custom;

    D(bug("+ Setting up copperlist.\n"));
    if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC | MEMF_CLEAR))
    {
        CINIT(ucl, COMMANDS);
        CWAIT(ucl, 0, 0);
        CMOVE(ucl, custom.intreq, INTF_SETCLR | INTF_COPER);
        CEND(ucl);

        Forbid();
        vp->UCopIns = ucl;
        Permit();

        RethinkDisplay();

        return(TRUE);
    }
    D(bug("Error: Out of memory!\n"));
    return(FALSE);
}

/*===============================================*/

BOOL addCopperIs(void (*copperIs)(void), struct Interrupt *is, struct copperData *cd, struct ViewPort *vp)
{
    D(bug("+ Setting up copper interrupt.\n"));
    if ((cd->signal = AllocSignal(-1)) != -1)
    {
        cd->task = FindTask(NULL);
        cd->vp   = vp;

        is->is_Code = copperIs;
        is->is_Data = (APTR)cd;
        is->is_Node.ln_Pri = PRI;

        AddIntServer(INTB_COPER, is);

        return(TRUE);
    }
    D(bug("Error: Out of signals!\n"));
    return(FALSE);
}

VOID remCopperIs(struct Interrupt *is)
{
    D(bug("- Removing copper interrupt.\n"));
    RemIntServer(INTB_COPER, is);

    FreeSignal(((struct copperData *)is->is_Data)->signal);
}

/*===============================================*/

/* EOF */
