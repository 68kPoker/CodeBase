
/* $Header: Work:Magazyn/RCS/Screen.c,v 1.3 12/.0/.2 .0:.1:.1 Robert Exp $ */

#include "Screen.h"

#include <exec/memory.h>
#include <intuition/screens.h>
#include <graphics/gfxmacros.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>

#include <clib/diskfont_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

__far extern struct Custom custom;

extern void myCopper(void);

LONG openScreen(struct screen *s)
{
    struct screenParam *sp;
    struct TextAttr *ta = NULL;
    struct Rectangle clip = { 0, 0, 319, 255 }, *dclip = &clip;
    ULONG modeID = LORES_KEY;
    UBYTE depth = DEPTH;
    ULONG *colors = NULL;
    struct Screen *scr;

    s->tf = NULL;

    if (sp = s->sp)
    {
        if (!(s->tf = OpenDiskFont(ta = sp->ta)))
        {
            printf("Couldn't open %s size %d!\n", ta->ta_Name, ta->ta_YSize);
            return(FALSE);
        }
        dclip  = &sp->dclip;
        modeID = sp->modeID;
        depth  = sp->depth;
        colors = sp->colors;
    }

    if (s->s = scr = OpenScreenTags(NULL,
        SA_BitMap,      s->bm[0],
        ta ? SA_Font : TAG_IGNORE, ta,
        SA_DClip,       dclip,
        SA_Depth,       depth,
        SA_DisplayID,   modeID,
        colors ? SA_Colors32 : TAG_IGNORE, colors,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Title,       "Gearworks Screen",
        TAG_DONE))
    {
        struct ViewPort *vp = &scr->ViewPort;
        struct copper *cop = &s->cop;

        cop->task = FindTask(NULL);
        cop->vp = vp;

        if ((cop->signal = AllocSignal(-1)) != -1)
        {
            struct UCopList *ucl;

            if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
            {
                struct Interrupt *is = &s->copIs;
                struct DBufInfo *dbi;

                CINIT(ucl, 3);
                CWAIT(ucl, 0, 0);
                CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                CEND(ucl);

                Forbid();
                vp->UCopIns = ucl;
                Permit();

                RethinkDisplay();

                is->is_Code = myCopper;
                is->is_Data = (APTR)cop;
                is->is_Node.ln_Pri = 0;

                AddIntServer(INTB_COPER, is);

                if (s->dbi = dbi = AllocDBufInfo(vp))
                {
                    if (s->safePort = CreateMsgPort())
                    {
                        dbi->dbi_SafeMessage.mn_ReplyPort = s->safePort;
                        s->safe = TRUE;
                        s->frame = 1;

                        return(TRUE);
                    }
                    FreeDBufInfo(dbi);
                }

                RemIntServer(INTB_COPER, is);
            }
            FreeSignal(cop->signal);
        }
        CloseScreen(scr);
    }

    if (s->tf)
    {
        CloseFont(s->tf);
    }
    return(FALSE);
}

void closeScreen(struct screen *s)
{
    struct MsgPort *mp = s->safePort;

    if (!s->safe)
    {
        while (!GetMsg(mp))
        {
            WaitPort(mp);
        }
    }

    DeleteMsgPort(mp);
    FreeDBufInfo(s->dbi);

    RemIntServer(INTB_COPER, &s->copIs);

    FreeSignal(s->cop.signal);
    CloseScreen(s->s);

    if (s->tf)
    {
        CloseFont(s->tf);
    }
}

void safeToDraw(struct screen *s)
{
    struct MsgPort *mp = s->safePort;

    if (!s->safe)
    {
        while (!GetMsg(mp))
        {
            WaitPort(mp);
        }
        s->safe = TRUE;
    }
}

void changeScreen(struct screen *s)
{
    if (s->safe)
    {
        WORD frame = s->frame;

        ChangeVPBitMap(&s->s->ViewPort, s->bm[frame], s->dbi);
        s->safe = FALSE;
        s->frame ^= 1;
    }
}
