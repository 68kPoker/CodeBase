
/* $Id$ */

#include "screen.h"

#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <exec/memory.h>
#include <intuition/screens.h>
#include <graphics/gfxmacros.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>

__far extern struct Custom custom;

BOOL init_copper(screen *s)
{
    copper_data      *cd  = &s->cis_data;
    struct Interrupt *cis = &s->cis;

    if ((cd->signal = AllocSignal(-1)) != -1)
    {
        struct UCopList *ucl;

        cd->vp   = &s->s->ViewPort;
        cd->task = FindTask(NULL);

        cis->is_Code = my_copper;
        cis->is_Data = (APTR)cd;
        cis->is_Node.ln_Pri  = COPERINTS_PRI;
        cis->is_Node.ln_Name = "GWS";

        if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
        {
            CINIT(ucl, COPERLIST_LEN);
            CWAIT(ucl, 0, 0);
            CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
            CEND(ucl);

            Forbid();
            cd->vp->UCopIns = ucl;
            Permit();

            RethinkDisplay();

            AddIntServer(INTB_COPER, cis);
            return(TRUE);
        }
        FreeSignal(cd->signal);
    }
    return(FALSE);
}

void free_copper(screen *s)
{
    RemIntServer(INTB_COPER, &s->cis);
    FreeSignal(s->cis_data.signal);
}

BOOL open_screen(screen *s)
{
    struct Rectangle dclip =
    {
        0, 0, 319, 255
    };
    UBYTE depth  = 5;
    ULONG modeID = LORES_KEY;

    if (s->s = OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_Depth,       depth,
        SA_DisplayID,   modeID,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE))
    {
        if (s->sb[0] = AllocScreenBuffer(s->s, NULL, SB_SCREEN_BITMAP))
        {
            if (s->sb[1] = AllocScreenBuffer(s->s, NULL, 0))
            {
                if (s->sp = CreateMsgPort())
                {
                    s->sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = s->sp;
                    s->sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = s->sp;
                    s->frame = 1;
                    s->safe  = TRUE;

                    if (init_copper(s))
                    {
                        return(TRUE);
                    }
                    DeleteMsgPort(s->sp);
                }
                FreeScreenBuffer(s->s, s->sb[1]);
            }
            FreeScreenBuffer(s->s, s->sb[0]);
        }
        CloseScreen(s->s);
    }
    return(FALSE);
}

void close_screen(screen *s)
{
    free_copper(s);

    if (!s->safe)
    {
        while (!GetMsg(s->sp))
        {
            WaitPort(s->sp);
        }
    }

    DeleteMsgPort(s->sp);
    FreeScreenBuffer(s->s, s->sb[1]);
    FreeScreenBuffer(s->s, s->sb[0]);
    CloseScreen(s->s);
}
