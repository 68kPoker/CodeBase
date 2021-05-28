
#include "System.h"

#include <intuition/screens.h>
#include <clib/intuition_protos.h>
#include <clib/utility_protos.h>

BOOL openScreen(struct systemInfo *si, ULONG tag1, ...)
{
    struct Rectangle dclip = { 0, 0, 319, 255 };
    struct TagItem tags[] =
    {
        SA_DClip,       (ULONG)&dclip,
        SA_Depth,       5,
        SA_DisplayID,   LORES_KEY,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_Exclusive,   TRUE,
        SA_BackFill,    (ULONG)LAYERS_NOBACKFILL,
        SA_Title,       (ULONG)"Magazyn",
        TAG_DONE
    };

    ApplyTagChanges(tags, (struct TagItem *)&tag1);

    if (si->s = OpenScreenTagList(NULL, tags))
    {
        if (addUserCopList(si->s))
        {
            if (addCopperInterrupt(&si->s->ViewPort, &si->is, &si->ci))
            {
                return(TRUE);
            }
        }
        CloseScreen(si->s);
    }
    return(FALSE);
}

void closeScreen(struct systemInfo *si)
{
    remCopperInterrupt(&si->is);
    CloseScreen(si->s);
}
