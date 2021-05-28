
#include <dos/dos.h>
#include <graphics/rastport.h>

#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#include "View.h"

typedef struct RastPort RPORT;

int main(void)
{
    VPACK vpack[2];
    VIEW *view = saveView();
    RPORT rp;
    UBYTE text[5];

    if (initView(&vpack[0], LORES_KEY, 320, 256, 5))
    {
        if (initView(&vpack[1], LORES_KEY, 320, 256, 5))
        {
            WORD i;
            UBYTE toggle = 0;

            InitRastPort(&rp);

            for (i = 0; i < 100; i++)
            {
                rp.BitMap = vpack[toggle].ri.BitMap;

                Move(&rp, 0, rp.Font->tf_Baseline);
                SetAPen(&rp, 2);
                sprintf(text, "%4d", i);
                Text(&rp, text, 4);

                LoadView(&vpack[toggle].view);
                WaitTOF();
                toggle ^= 1;
            }

            LoadView(view);
            WaitTOF();
            WaitTOF();
            freeView(&vpack[1]);
        }
        freeView(&vpack[0]);
    }
    return(RETURN_OK);
}
