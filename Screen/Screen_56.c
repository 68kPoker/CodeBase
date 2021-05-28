
#include <intuition/screens.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>

#include "Draw.h"

struct Screen *openScreen(void)
{
    struct Screen *s;
    struct Rectangle dclip = { 0, 0, 319, 255 };

    if (s = OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_Depth,       5,
        SA_DisplayID,   LORES_KEY,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_Title,       "Magazyn",
        TAG_DONE))
    {
        return s;
    }
    return NULL;
}

int main(void)
{
    struct Screen *s;

    if (s = openScreen())
    {
        struct RastPort *rp = &s->RastPort;

        drawLine(rp, 16, 16, 64, 32);

        Delay(400);
        CloseScreen(s);
    }
    return 0;
}
