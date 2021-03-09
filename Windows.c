
#include "Windows.h"
#include "Gadgets.h"

#include <intuition/intuition.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>

struct TagItem *getWindowTags(struct Screen *s)
{
    static struct TagItem tags[] =
    {
        { WA_CustomScreen,  0 },
        { WA_Left,          0 },
        { WA_Top,           0 },
        { WA_Width,         320 },
        { WA_Height,        256 },
        { WA_Backdrop,      TRUE },
        { WA_Borderless,    TRUE },
        { WA_Activate,      TRUE },
        { WA_RMBTrap,       TRUE },
        { WA_IDCMP,         0 },
        { WA_SimpleRefresh, 0 },
        { WA_BackFill,      (ULONG)LAYERS_NOBACKFILL },
        { WA_ReportMouse,   TRUE },
        { TAG_DONE }
    };

    tags[0].ti_Data = (ULONG)s;

    return(tags);
}

struct windowInfo *openWindow(struct TagItem *base, ULONG tag1, ...)
{
    struct windowInfo *wi;

    if (wi = AllocMem(sizeof(*wi), MEMF_PUBLIC|MEMF_CLEAR))
    {
        ApplyTagChanges(base, (struct TagItem *)&tag1);

        if (wi->window = OpenWindowTagList(NULL, base))
        {
            NewList((struct List *)&wi->gadList);

            wi->window->UserData = (APTR)wi;
            return(wi);
        }
        FreeMem(wi, sizeof(*wi));
    }
    return(NULL);
}

void closeWindow(struct windowInfo *wi)
{
    CloseWindow(wi->window);
    FreeMem(wi, sizeof(*wi));
}
