
#include <intuition/intuition.h>

#include <clib/intuition_protos.h>

#include "Window.h"

BOOL openBDWindow (CWindow *p, CScreen *s)
{
    if (p->Window = OpenWindowTags(NULL,
        WA_CustomScreen,    s->Screen,
        WA_Left,            0,
        WA_Top,             0,
        WA_Width,           s->Screen->Width,
        WA_Height,          s->Screen->Height,
        WA_Backdrop,        TRUE,
        WA_Borderless,      TRUE,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_SimpleRefresh,   TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_IDCMP,           IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_RAWKEY | IDCMP_INTUITICKS,
        WA_ReportMouse,     TRUE,
        TAG_DONE))
    {
        return(TRUE);
    }
    return(FALSE);
}

VOID closeWindow (CWindow *p)
{
    CloseWindow(p->Window);
}
