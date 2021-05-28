
#include <stdio.h>

#include <dos/dos.h>

#include <intuition/screens.h>
#include <intuition/intuition.h>

#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/utility_protos.h>
#include <clib/intuition_protos.h>

#define DEPTH 5
#define TITLE "Magazyn"

struct Screen *openScreen(Tag tag1, ...)
{
    struct Rectangle dclip =
    {
        0, 0, 319, 255
    };

    struct TagItem tags[] =
    {
        SA_DClip,       (ULONG)&dclip,
        SA_Depth,       DEPTH,
        SA_DisplayID,   LORES_KEY,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_Title,       (ULONG)TITLE,
        SA_BackFill,    (ULONG)LAYERS_NOBACKFILL,
        TAG_MORE,       (ULONG)&tag1
    };

    struct Screen *s;

    ApplyTagChanges(tags, (struct TagItem *)&tag1);

    return(s = OpenScreenTagList(NULL, tags));
}

struct Window *openWindow(Tag tag1, ...)
{
    struct TagItem tags[] =
    {
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           320,
        WA_Height,          256,
        WA_AutoAdjust,      FALSE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        (ULONG)LAYERS_NOBACKFILL,
        WA_NoCareRefresh,   TRUE,
        WA_IDCMP,           0,
        WA_ReportMouse,     TRUE,
        TAG_MORE,           (ULONG)&tag1
    };

    struct Window *w;

    ApplyTagChanges(tags, (struct TagItem *)&tag1);

    return(w = OpenWindowTagList(NULL, tags));
}
