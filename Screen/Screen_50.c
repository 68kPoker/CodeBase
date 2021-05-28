
#include <intuition/screens.h>

#include <clib/intuition_protos.h>

#include "Screen.h"

BOOL openScreen (CScreen *p)
{
    struct Rectangle dclip =
    {
        0, 0, 319, 255
    };
    const UBYTE depth = 5;
    const ULONG modeID = LORES_KEY;

    if (p->Screen = OpenScreenTags(NULL,
        SA_DClip,       &dclip,
        SA_Depth,       depth,
        SA_DisplayID,   modeID,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Title,       "Gear Works Screen",
        SA_ShowTitle,   FALSE,
        TAG_DONE))
    {
        return(TRUE);
    }
    return(FALSE);
}

VOID closeScreen (CScreen *p)
{
    CloseScreen(p->Screen);
}
