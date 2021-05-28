
#include <intuition/intuition.h>
#include <clib/exec_protos.h>

#include "Game.h"

BOOL initGame (CGame *p)
{
    if (openScreen(&p->Screen))
    {
        if (openBDWindow(&p->Window, &p->Screen))
        {
            clearBoard(p->BoardGad.Board);
            initBoardGadget(&p->BoardGad, &p->Window);
            return(TRUE);
        }
        closeScreen(&p->Screen);
    }
    return(FALSE);
}

VOID closeGame (CGame *p)
{
    closeWindow (&p->Window);
    closeScreen (&p->Screen);
}

VOID playGame (CGame *p)
{
    struct MsgPort *mp = p->Window.Window->UserPort;
    ULONG signals[] = { 1L << mp->mp_SigBit };
    ULONG total = signals [0];
    BOOL done = FALSE;

    while (!done)
    {
        ULONG result = Wait(total);

        if (result & signals [0])
        {
            struct IntuiMessage *msg;

            while ((!done) && (msg = (struct IntuiMessage *)GetMsg(mp)))
            {
                if (msg->Class == IDCMP_RAWKEY && msg->Code == ESC_KEY)
                {
                    done = TRUE;
                }
                else if (msg->Class == IDCMP_INTUITICKS)
                {
                    drawBoard(&p->BoardGad);
                }
                else
                {
                    editBoard(&p->BoardGad, msg);
                }
                ReplyMsg((struct Message *)msg);
            }
        }
    }
}

int main (void)
{
    CGame game;

    if (initGame(&game))
    {
        playGame(&game);
        closeGame(&game);
    }
    return(0);
}
