
#include <exec/types.h>
#include <exec/interrupts.h>
#include <exec/memory.h>

#include <hardware/intbits.h>
#include <hardware/custom.h>

#include <graphics/gfxmacros.h>

#include <intuition/screens.h>

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#define COMMANDS    3 /* Copper list len */

struct copperInfo
{
    struct ViewPort     *vp;
    UWORD               signal;
    struct Task         *task;
};

struct screenInfo
{
    struct Screen       *s;
    struct BitMap       *bm[2];
    struct DBufInfo     *dbi;
    UWORD               frame;
    BOOL                safe;
    struct Interrupt    is;
    struct copperInfo   ci;

    struct Region       *validate[2];
};

__far extern struct Custom custom;

extern void copperInterrupt(void);

BOOL screenOpen(struct screenInfo *si, UWORD rasW, UWORD rasH, UBYTE rasD)
{
    struct Rectangle dclip = { 0, 0, 319, 255 };

    if (si->bm[0] = AllocBitMap(rasW, rasH, rasD, BMF_DISPLAYABLE|BMF_INTERLEAVED, NULL))
    {
        if (si->bm[1] = AllocBitMap(rasW, rasH, rasD, BMF_DISPLAYABLE|BMF_INTERLEAVED, NULL))
        {
            /* Clear or init bitmaps */
            BltClear(si->bm[0]->Planes[0], 0x3, (si->bm[0]->Rows << 16) | si->bm[0]->BytesPerRow);
            BltClear(si->bm[1]->Planes[0], 0x3, (si->bm[1]->Rows << 16) | si->bm[1]->BytesPerRow);

            if (si->s = OpenScreenTags(NULL,
                SA_DClip,       &dclip,
                SA_BitMap,      si->bm[0],
                SA_Title,       "Magazyn",
                SA_Quiet,       TRUE,
                SA_ShowTitle,   FALSE,
                SA_BackFill,    LAYERS_NOBACKFILL,
                SA_DisplayID,   LORES_KEY,
                TAG_DONE))
            {
                struct ViewPort *vp = &si->s->ViewPort;

                if (si->dbi = AllocDBufInfo(vp))
                {
                    struct MsgPort *mp;

                    si->frame = 1;
                    si->safe = TRUE;

                    if (mp = CreateMsgPort())
                    {
                        struct UCopList *ucl;

                        si->dbi->dbi_SafeMessage.mn_ReplyPort = mp;

                        if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
                        {
                            struct copperInfo *ci = &si->ci;

                            CINIT(ucl, COMMANDS);
                            CWAIT(ucl, 0, 0);
                            CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
                            CEND(ucl);

                            Forbid();
                            vp->UCopIns = ucl;
                            Permit();

                            RethinkDisplay();

                            if ((ci->signal = AllocSignal(-1)) != -1)
                            {
                                struct Interrupt *is = &si->is;

                                ci->task = FindTask(NULL);
                                ci->vp = vp;

                                is->is_Code = copperInterrupt;
                                is->is_Data = (APTR)ci;
                                is->is_Node.ln_Pri = 0;
                                is->is_Node.ln_Name = "Magazyn";

                                if (si->validate[0] = NewRegion())
                                {
                                    if (si->validate[1] = NewRegion())
                                    {
                                        AddIntServer(INTB_COPER, is);

                                        return(TRUE);
                                    }
                                    DisposeRegion(si->validate[0]);
                                }
                                FreeSignal(ci->signal);
                            }
                        }
                        DeleteMsgPort(mp);
                    }
                    FreeDBufInfo(si->dbi);
                }
                CloseScreen(si->s);
            }
            FreeBitMap(si->bm[1]);
        }
        FreeBitMap(si->bm[0]);
    }
    return(FALSE);
}

void screenClose(struct screenInfo *si)
{
    struct MsgPort *mp = si->dbi->dbi_SafeMessage.mn_ReplyPort;

    RemIntServer(INTB_COPER, &si->is);

    DisposeRegion(si->validate[1]);
    DisposeRegion(si->validate[0]);

    FreeSignal(si->ci.signal);

    if (!si->safe)
    {
        while (!GetMsg(mp))
        {
            WaitPort(mp);
        }
    }
    DeleteMsgPort(mp);
    FreeDBufInfo(si->dbi);
    CloseScreen(si->s);
    FreeBitMap(si->bm[1]);
    FreeBitMap(si->bm[0]);
}
