
#include "Screen.h"

#include <intuition/screens.h>
#include <dos/dos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

void testScreen(struct screen *s)
{
    BOOL done = FALSE;
    WORD counter = 0;
    struct RastPort *rp = &s->s->RastPort;
    WORD frame;
    UBYTE text[10];

    while (!done)
    {
        Wait(1L << s->cop.signal);

        changeScreen(s);

        WaitPort(s->safePort);

        safeToDraw(s);

        frame = s->frame;

        rp->BitMap = s->bm[frame];

        Move(rp, 0, s->tf->tf_Baseline);
        SetABPenDrMd(rp, 2, 0, JAM2);
        sprintf(text, "%4d ", counter);
        Text(rp, text, 5);
        if (++counter == 200)
        {
            done = TRUE;
        }
    }
}

int main()
{
    struct TextAttr ta =
    {
        "centurion.font", 9, FS_NORMAL, FPF_DISKFONT|FPF_DESIGNED
    };
    struct screen s;
    LONG result = FALSE;

    if (s.bm[0] = AllocBitMap(320, 256, DEPTH, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
    {
        if (s.bm[1] = AllocBitMap(320, 256, DEPTH, BMF_DISPLAYABLE|BMF_INTERLEAVED|BMF_CLEAR, NULL))
        {
            { struct screenParam sp =
            {
                { 0, 0, 319, 255 },
                LORES_KEY,
                DEPTH,
                &ta,
                NULL
            };

            s.sp = &sp;
            result = openScreen(&s);
            }

            if (result)
            {
                testScreen(&s);
                closeScreen(&s);
            }
            FreeBitMap(s.bm[1]);
        }
        FreeBitMap(s.bm[0]);
    }

    return(RETURN_OK);
}
