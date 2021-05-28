
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#include "Screen.h"
#include "Windows.h"

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

int main()
{
    struct screenInfo si;

    if (openScreen(&si, LORES_KEY, 5))
    {
        struct windowInfo wi;

        SetRGB32(&si.s->ViewPort, 1, RGB(255), RGB(255), RGB(255));

        if (openWindow(&wi, &si, 0, 0, 160, 128, IDCMP_RAWKEY))
        {
            WORD i;
            for (i = 0; i < 50; i++)
            {
                WaitTOF();
                SetRast(wi.w->RPort, 1);
                moveWindow(&wi, 1, 1);
            }
            Delay(400);
            closeWindow(&wi);
        }
        closeScreen(&si);
    }
    return (RETURN_OK);
}
