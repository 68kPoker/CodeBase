
/* $Log$ */

#include <stdio.h>
#include <intuition/intuition.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>

#include "Main.h"

BOOL getMainData(struct mainData *md)
{
    if (getScreenData(&md->sd, RASWIDTH, RASHEIGHT, RASDEPTH, MODEID))
    {
        return(TRUE);
    }
    return(FALSE);
}

void dropMainData(struct mainData *md)
{
    dropScreenData(&md->sd);
}

BOOL initAnimation(struct mainData *md)
{
    md->ti.text = "Amiga";
    md->ti.x = 0;
    md->ti.y = 8;
    md->ad.info = &md->ti;
    md->ad.draw = drawText;
}

BOOL run(struct mainData *md)
{
    ULONG signals[SIG_SOURCES], total, result;
    WORD i;
    BOOL done = FALSE;
    BOOL held = TRUE;

    signals[SIG_SAFE] = 1L << md->sd.mp[0]->mp_SigBit;
    signals[SIG_DISP] = 1L << md->sd.mp[1]->mp_SigBit;

    total = 0L;
    for (i = 0; i < SIG_SOURCES; i++)
    {
        total |= signals[i];
    }

    /* Set screen buffer */
    md->sd.frame = 1;

    initAnimation(md);

    while (!done)
    {
        if (!held)
        {
            result = Wait(total);
        }
        else
        {
            WaitTOF();
            result = signals[SIG_DISP];
        }

        if (result & signals[SIG_SAFE])
        {
            static WORD counter = 0;

            if (!md->sd.safe[0])
            {
                while (!GetMsg(md->sd.mp[0]))
                {
                    WaitPort(md->sd.mp[0]);
                }
            }

            /* Safe to write */
            md->sd.safe[0] = TRUE;

            /* Write here */
            if (counter < 100)
            {
                UBYTE text[5];

                struct RastPort *rp = &md->sd.s->RastPort;
                struct TextFont *tf = rp->Font;

                rp->BitMap = md->sd.sb[md->sd.frame]->sb_BitMap;

                sprintf(text, "%4d", counter);
                SetRGB4(&md->sd.s->ViewPort, 1, 15, 15, 15);
                SetAPen(rp, 1);
                Move(rp, 0, tf->tf_Baseline);
                Text(rp, text, 4);

                counter++;
            }
            else
            {
                done = TRUE;
            }
        }

        if (result & signals[SIG_DISP])
        {
            if (!md->sd.safe[1])
            {
                while (!GetMsg(md->sd.mp[1]))
                {
                    WaitPort(md->sd.mp[1]);
                }
            }

            /* Displayed at least once */
            md->sd.safe[1] = TRUE;

            /* Change here */
            held = !ChangeScreenBuffer(md->sd.s, md->sd.sb[md->sd.frame]);

            if (!held)
            {
                md->sd.safe[0] = md->sd.safe[1] = FALSE;
                md->sd.frame ^= 1;
            }
        }
    }
    return(TRUE);
}

int main()
{
    struct mainData md = { 0 };

    if (getMainData(&md))
    {
        run(&md);
        dropMainData(&md);
    }
    return(0);
}
