
#include <intuition/screens.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <graphics/gfxmacros.h>

#include <clib/utility_protos.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#include "Screen.h"

extern __far struct Custom custom;
extern void myCopper(void);

struct Rectangle dclip = { 0, 0, 319, 255 };
struct TextAttr ta = { "centurion.font", 9, FS_NORMAL, FPF_DISKFONT|FPF_DESIGNED };

struct TagItem screenTags[] =
{
    SA_DClip,       (ULONG)&dclip,
    SA_DisplayID,   LORES_KEY,
    SA_Depth,       5,
    SA_Quiet,       TRUE,
    SA_Exclusive,   TRUE,
    SA_ShowTitle,   FALSE,
    SA_BackFill,    (ULONG)LAYERS_NOBACKFILL,
    SA_Title,       (ULONG)"Magazyn",
    SA_Font,        (ULONG)&ta,
    TAG_DONE
};

/*
** Easy open screen using optional Tag modifiers.
*/
struct Screen *openScreen(struct TagItem *taglist, ULONG tag1, ...)
{
    struct Screen *s;

    ApplyTagChanges(taglist, (struct TagItem *)&tag1);

    if (s = OpenScreenTagList(NULL, taglist))
    {
        return(s);
    }
    return(NULL);
}

/*
** Add double buffering
*/
BOOL addDBuf(struct Screen *s, struct screenInfo *si)
{
    struct Interrupt *is = &si->is;
    struct copperInfo *ci = &si->ci;
    struct UCopList *ucl;

    si->s = s;
    s->UserData = (APTR)si;
    if (si->sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP))
    {
        if (si->sb[1] = AllocScreenBuffer(s, NULL, 0))
        {
            if (si->mp = CreateMsgPort())
            {
                si->sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = si->mp;
                si->sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = si->mp;
                si->safe = TRUE;
                si->frame = 1;

                is->is_Code = myCopper;
                is->is_Data = (APTR)ci;
                is->is_Node.ln_Pri = 0;

                if ((ci->signal = AllocSignal(-1)) != -1)
                {
                    ci->vp = &s->ViewPort;
                    ci->task = FindTask(NULL);

                    if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                    {
                        CINIT(ucl, 3);
                        CWAIT(ucl, 0, 0);
                        CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                        CEND(ucl);

                        Forbid();
                        s->ViewPort.UCopIns = ucl;
                        Permit();

                        AddIntServer(INTB_COPER, is);

                        return(TRUE);
                    }
                    FreeSignal(ci->signal);
                }
                DeleteMsgPort(si->mp);
            }
            FreeScreenBuffer(s, si->sb[1]);
        }
        FreeScreenBuffer(s, si->sb[0]);
    }
    return(FALSE);
}

/*
** Close screen and optional info.
*/
void closeScreen(struct Screen *s)
{
    struct screenInfo *si = (struct screenInfo *)s->UserData;

    if (si)
    {
        RemIntServer(INTB_COPER, &si->is);
        FreeSignal(si->ci.signal);

        if (!si->safe)
        {
            while (!GetMsg(si->mp))
            {
                WaitPort(si->mp);
            }
        }
        DeleteMsgPort(si->mp);
        FreeScreenBuffer(s, si->sb[1]);
        FreeScreenBuffer(s, si->sb[0]);
    }

    CloseScreen(s);
}
