
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/graphics_protos.h>

#include "System.h"
#include "Joystick.h"
#include "Data.h"

enum
{
    INPUT_SAFE,
    INPUT_DISP,
    INPUT_IDCMP,
    INPUT_JOY,
    INPUTS
};

#define ESC_KEY 0x45

BOOL handleidcmp(struct system *sys);
void handledraw(struct system *sys);
void handlechange(struct system *sys);

void drawskin(struct BitMap *bm, struct RastPort *rp);

extern BOOL initgame(struct system *sys)
{
    if (sys->gfx = openGraphics("Graphics/Graphics.pic", sys->s, FALSE))
    {
        sys->drawback = drawskin;
        return(TRUE);
    }
    return(FALSE);
}

extern void playgame(struct system *sys)
{
    ULONG signals[INPUTS], total;
    WORD i;
    BOOL done = FALSE;
    BOOL init = TRUE;

    signals[INPUT_SAFE] = 1L << sys->dbufports[0]->mp_SigBit;
    signals[INPUT_DISP] = 1L << sys->dbufports[1]->mp_SigBit;
    signals[INPUT_IDCMP] = 1L << sys->w->UserPort->mp_SigBit;
    signals[INPUT_JOY] = 1L << sys->joyio->io_Message.mn_ReplyPort->mp_SigBit;

    total = 0L;
    for (i = 0; i < INPUTS; i++)
        total |= signals[i];

    while (!done)
    {
        ULONG result;

        if (!init)
            result = Wait(total);
        else
        {
            result = signals[INPUT_DISP];
            init = FALSE;
        }

        if (result & signals[INPUT_SAFE])
        {
            if (!sys->safe[0])
                while (!GetMsg(sys->dbufports[0]))
                    WaitPort(sys->dbufports[0]);

            sys->safe[0] = TRUE;

            handledraw(sys);
        }

        if (result & signals[INPUT_DISP])
        {
            if (!sys->safe[1])
                while (!GetMsg(sys->dbufports[1]))
                    WaitPort(sys->dbufports[1]);

            sys->safe[1] = TRUE;

            handlechange(sys);
        }

        if (result & signals[INPUT_IDCMP])
        {
            done = handleidcmp(sys);
        }

        if (result & signals[INPUT_JOY])
        {
            struct RastPort *rp = sys->w->RPort;
            static WORD pen = 1;

            if (sys->joyie.ie_Code == IECODE_LBUTTON)
                ;

            readevent(sys->joyio, &sys->joyie);
        }
    }
}

void drawskin(struct BitMap *bm, struct RastPort *rp)
{
    BltBitMapRastPort(bm, 0, 0, rp, 0, 0, 320, 32, 0xc0);
    BltBitMapRastPort(bm, 0, 32, rp, 0, 32, 34, 224, 0xc0);
}

void drawboard(struct BitMap *bm, struct RastPort *rp)
{
    BltBitMapRastPort(bm, 32, 32, rp, 32, 32, 288, 224, 0xc0);
}

void handledraw(struct system *sys)
{
    struct RastPort *rp = sys->w->RPort;
    struct TextFont *tf = rp->Font;
    static WORD counter = 0;
    WORD frame = sys->frame;

    rp->BitMap = sys->sb[frame]->sb_BitMap;

    if (!sys->backDrawn[frame])
    {
        struct BitMap *bm = (struct BitMap *)sys->s->UserData;

        sys->drawback(bm, rp);

        sys->backDrawn[frame] = TRUE;
    }

    Move(rp, 113, 26);
    SetABPenDrMd(rp, 24, 0, JAM1);
    Text(rp, "Loading...", 10);

    SetAPen(rp, 21);
    Move(rp, 112, 25);
    Text(rp, "Loading...", 10);

    RectFill(rp, 146, 20, 146 + counter, 26);
    if (counter < 165)
        counter++;
    else if (sys->drawback != drawboard)
    {
        sys->drawback = drawboard;
        sys->backDrawn[0] = sys->backDrawn[1] = FALSE;
    }
}

void handlechange(struct system *sys)
{
    WORD frame = sys->frame;

    WaitBlit();
    while (!ChangeScreenBuffer(sys->s, sys->sb[frame]))
    {
        WaitTOF();
    }

    sys->safe[0] = sys->safe[1] = FALSE;
    sys->frame = frame ^ 1;
}

BOOL handleidcmp(struct system *sys)
{
    struct IntuiMessage *msg;
    BOOL done = FALSE;

    while (msg = GT_GetIMsg(sys->w->UserPort))
    {
        if (msg->Class == IDCMP_RAWKEY)
        {
            if (msg->Code == ESC_KEY)
            {
                done = TRUE;
            }
        }
        GT_ReplyIMsg(msg);
    }
    return(done);
}

extern void closegame(struct system *sys)
{
    DisposeDTObject(sys->gfx);
}
