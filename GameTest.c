
#include "Screen.h"
#include "Windows.h"
#include "Gadgets.h"

#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>

int main(void)
{
    struct screenInfo *si;

    if (si = openScreen(getScreenTags(), TAG_DONE))
    {
        struct windowInfo *wi;

        if (wi = openWindow(getWindowTags(si->screen),
            WA_IDCMP,   IDCMP_GADGETUP,
            TAG_DONE))
        {
            struct gadgetInfo *gi;

            if (gi = newGadget())
            {
                setGadgetBox(gi, 0, 0, 32, 32);
                setGadgetID(gi, 3);

                AddTail((struct List *)&wi->gadList, (struct Node *)gi);

                linkGadgets(&wi->gadList);

                AddGList(wi->window, &gi->gad, -1, -1, NULL);
                WaitPort(wi->window->UserPort);
                RemoveGList(wi->window, &gi->gad, -1);

                disposeGadget(gi);
            }
            closeWindow(wi);
        }
        closeScreen(si);
    }
    return(0);
}
