
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>

struct Window *openBackdrop(struct Screen *s)
{
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           320,
        WA_Height,          256,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_IDCMP,           IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
        WA_ReportMouse,     TRUE,
        TAG_DONE)) {
        return(w);
    }
    return(NULL);
}
