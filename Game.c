
/* Magazyn */

/* $Id$ */

#include <stdio.h>

#include <dos/dos.h>
#include <intuition/intuition.h>
#include <graphics/gfxmacros.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

#define DEPTH 5

enum
{
    SAFE_SIG,
    COPPER_SIG
};

__far extern struct Custom custom;
extern void myCopper(void);

struct screenUD
{
    struct Screen *s;
    struct ScreenBuffer *sb[2];
    struct MsgPort *sp;
    BOOL safe;
    UWORD frame;
    struct Interrupt is;
    BOOL added;
    struct copperUD
    {
        struct ViewPort *vp;
        WORD signal;
        struct Task *task;
    } cop;
};

int main(void);
BOOL openScreen(void);
void closeScreen(struct screenUD *sud);

BOOL loop(struct screenUD *sud)
{
    ULONG total = 0;
    WORD counter = 0;
    ULONG signals[] =
    {
        1L << sud->sp->mp_SigBit,
        1L << sud->cop.signal
    };
    BOOL done = FALSE;

    total = signals[SAFE_SIG]|signals[COPPER_SIG];

    while (!done)
    {
        ULONG result = Wait(total);

        if (result & signals[SAFE_SIG])
        {
            /* Safe to draw */
            struct RastPort *rp = &sud->s->RastPort;
            struct TextFont *font = rp->Font;
            UWORD frame = sud->frame;
            UBYTE text[5];

            if (!sud->safe)
            {
                while (!GetMsg(sud->sp))
                {
                    WaitPort(sud->sp);
                }
                sud->safe = TRUE;
            }

            rp->BitMap = sud->sb[frame]->sb_BitMap;

            sprintf(text, "%4d", counter);

            Move(rp, 0, font->tf_Baseline);
            SetAPen(rp, 1);
            Text(rp, text, 4);
        }

        if (result & signals[COPPER_SIG])
        {
            UWORD frame = sud->frame;

            if (sud->safe)
            {
                while (!ChangeScreenBuffer(sud->s, sud->sb[frame]))
                {
                    WaitTOF();
                }
                sud->frame = frame ^= 1;
                sud->safe = FALSE;

                counter++;
                if (counter == 200)
                    done = TRUE;
            }
        }
    }
    printf("%d\n", counter);
    return(TRUE);
}

void closeScreen(struct screenUD *sud)
{
    struct MsgPort *sp = sud->sp;
    struct Screen *s = sud->s;
    struct ScreenBuffer *sb;
    WORD signal = sud->cop.signal;

    if (sud->added) RemIntServer(INTB_COPER, &sud->is);
    if (signal != -1) FreeSignal(signal);
    if (sp)
    {
        if (!sud->safe)
        {
            while (!GetMsg(sp))
            {
                WaitPort(sp);
            }
        }
        DeleteMsgPort(sp);
    }
    if (sb = sud->sb[1]) FreeScreenBuffer(s, sb);
    if (sb = sud->sb[0]) FreeScreenBuffer(s, sb);
    if (s)               CloseScreen(s);
}

/* Open screen and proceed */
BOOL openScreen(void)
{
    struct screenUD sud = { 0 };
    static struct Rectangle dclip = { 0, 0, 319, 255 };
    const ULONG modeID = LORES_KEY;
    static UBYTE title[] = "Magazyn";

    struct Screen *s;
    struct MsgPort *sp;
    struct Interrupt *is = &sud.is;
    struct copperUD *cop = &sud.cop;
    struct UCopList *ucl;

    sud.cop.signal = -1; /* Signal not allocated */

    if (!(sud.s = s = OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_Depth,       DEPTH,
        SA_DisplayID,   modeID,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_Title,       title,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE)))
    {
        printf("Couldn't open screen!\n");
        return(FALSE);
    }

    if (!(sud.sb[0] = AllocScreenBuffer(s, NULL, SB_SCREEN_BITMAP)))
    {
        printf("Couldn't alloc screen buffer!\n");
        closeScreen(&sud);
        return(FALSE);
    }

    if (!(sud.sb[1] = AllocScreenBuffer(s, NULL, 0)))
    {
        printf("Couldn't alloc screen buffer!\n");
        closeScreen(&sud);
        return(FALSE);
    }

    if (!(sud.sp = sp = CreateMsgPort()))
    {
        printf("Couldn't create msgport!\n");
        closeScreen(&sud);
        return(FALSE);
    }

    sud.safe = TRUE;
    sud.frame = 1;

    sud.sb[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sp;
    sud.sb[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sp;

    cop->vp = &s->ViewPort;
    cop->task = FindTask(NULL);

    if ((cop->signal = AllocSignal(-1)) == -1)
    {
        printf("Couldn't alloc signal!\n");
        closeScreen(&sud);
        return(FALSE);
    }

    is->is_Code = myCopper;
    is->is_Data = cop;
    is->is_Node.ln_Pri = 0;

    if (!(ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR)))
    {
        printf("Out of memory!\n");
        closeScreen(&sud);
        return(FALSE);
    }

    CINIT(ucl, 3);
    CWAIT(ucl, 0, 0);
    CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
    CEND(ucl);

    Forbid();
    s->ViewPort.UCopIns = ucl;
    Permit();

    RethinkDisplay();

    AddIntServer(INTB_COPER, is);

    sud.added = TRUE;

    loop(&sud); /* Enter main loop */

    closeScreen(&sud);

    return(TRUE);
}

int main(void)
{
    if (!openScreen())
    {
        printf("Failure.\n");
        return(RETURN_FAIL);
    }
    return(RETURN_OK);
}
