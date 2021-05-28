
#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <graphics/gfxmacros.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "Screen.h"

extern __far struct Custom custom;

/* Common attributes */
struct TagItem wtags[] =
{
    WA_SimpleRefresh,   TRUE,
    WA_BackFill,        (ULONG)LAYERS_NOBACKFILL,
    WA_RMBTrap,         TRUE,
    WA_Borderless,      TRUE,
    TAG_DONE
};

UWORD pens[] = { ~0 };

/* openScreen: Open double-buffered screen with Copper interrupt,
 *             BitMaps s->bm[] must be allocated and prepared.
 *             Display Clip must be calculated.
 */
BOOL openScreen(struct screen *s, WORD minX, WORD minY, WORD maxX, WORD maxY, ULONG modeID, ULONG *pal)
{
    struct Rectangle dclip = { minX, minY, maxX, maxY };

    if (s->s = OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_BitMap,      s->bm[0],
        SA_Colors32,    pal,
        SA_DisplayID,   modeID,
        SA_Title,       "Gear Works Screen",
        SA_ShowTitle,   FALSE,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Pens,        pens,
        TAG_DONE))
    {
        if (s->sb[0] = AllocScreenBuffer(s->s, s->bm[0], 0))
        {
            if (s->sb[1] = AllocScreenBuffer(s->s, s->bm[1], 0))
            {
                if (s->mp = CreateMsgPort())
                {
                    s->safe = TRUE;
                    s->frame = 1;
                    s->sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = s->mp;
                    s->sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = s->mp;

                    s->cop.task = FindTask(NULL);

                    if ((s->cop.signal = AllocSignal(-1)) != (-1))
                    {
                        struct UCopList *ucl;

                        if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                        {
                            if (s->dri = GetScreenDrawInfo(s->s))
                            {
                                s->is.is_Code = myCopper;
                                s->is.is_Data = (APTR)&s->cop;
                                s->is.is_Node.ln_Pri = COPPER_PRI;
                                s->is.is_Node.ln_Name = "Gear Works Copper Interrupt";

                                AddIntServer(INTB_COPER, &s->is);

                                CINIT(ucl, COPPER_COMMANDS);
                                CWAIT(ucl, 0, 0);
                                CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                                CEND(ucl);

                                Forbid();
                                s->s->ViewPort.UCopIns = ucl;
                                Permit();

                                s->s->UserData = (APTR)s;

                                return(TRUE);
                            }
                            FreeMem(ucl, sizeof(*ucl));
                        }
                        FreeSignal(s->cop.signal);
                    }
                    DeleteMsgPort(s->mp);
                }
                FreeScreenBuffer(s->s, s->sb[1]);
            }
            FreeScreenBuffer(s->s, s->sb[0]);
        }
        CloseScreen(s->s);
    }
    return(FALSE);
}

/* obtainBitMap: Obtain current hidden buffer bitmap. */
struct BitMap *obtainBitMap(struct screen *s)
{
    return(s->sb[s->frame]->sb_BitMap);
}

/* safeToDraw: Ensure it's safe to draw. Wait if it's not. */
void safeToDraw(struct screen *s)
{
    if (!s->safe)
    {
        while (!GetMsg(s->mp))
        {
            WaitPort(s->mp);
        }
        s->safe = TRUE;
    }
}

/* changeScreen: Swap screen buffers. */
void changeScreen(struct screen *s)
{
    WORD frame = s->frame;

    WaitBlit();
    while (!ChangeScreenBuffer(s->s, s->sb[frame]))
    {
        WaitTOF();
    }

    s->frame = frame ^= 1;
    s->safe = FALSE;
}

void closeScreen(struct screen *s)
{
    RemIntServer(INTB_COPER, &s->is);
    FreeSignal(s->cop.signal);
    FreeScreenDrawInfo(s->s, s->dri);

    if (!s->safe)
    {
        while (!GetMsg(s->mp))
        {
            WaitPort(s->mp);
        }
    }

    DeleteMsgPort(s->mp);
    FreeScreenBuffer(s->s, s->sb[1]);
    FreeScreenBuffer(s->s, s->sb[0]);
    CloseScreen(s->s);
}

BOOL openWindow(struct window *w, struct Screen *s, WORD minX, WORD minY, WORD maxX, WORD maxY, BOOL backdrop, BOOL activate, ULONG idcmp, struct Gadget *gads)
{
    if (w->w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            minX,
        WA_Top,             minY,
        WA_Width,           maxX - minX + 1,
        WA_Height,          maxY - minY + 1,
        WA_Backdrop,        backdrop,
        WA_Activate,        activate,
        WA_IDCMP,           idcmp,
        WA_ReportMouse,     idcmp & IDCMP_MOUSEMOVE,
        WA_Gadgets,         gads,
        TAG_MORE,           wtags))
    {
        w->w->UserData = (APTR)w;
        return(TRUE);
    }
    return(FALSE);
}

void closeWindow(struct window *w)
{
    CloseWindow(w->w);
}
