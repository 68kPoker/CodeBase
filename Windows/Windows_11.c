
#include <intuition/intuition.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>

#include "Windows.h"

struct Window *openWindow(struct Screen *s, WORD left, WORD top, WORD width, WORD height, ULONG idcmp, BOOL backdrop)
{
    struct Window *w;

    if (w = OpenWindowTags(NULL,
        WA_CustomScreen,    s,
        WA_Left,            left,
        WA_Top,             top,
        WA_Width,           width,
        WA_Height,          height,
        WA_Borderless,      TRUE,
        WA_Backdrop,        backdrop,
        WA_Activate,        TRUE,
        WA_RMBTrap,         TRUE,
        WA_IDCMP,           idcmp,
        WA_ReportMouse,     TRUE,
        WA_BackFill,        LAYERS_NOBACKFILL,
        WA_SimpleRefresh,   TRUE,
        TAG_DONE))
    {
        if (newWindowData(w))
        {
            return(w);
        }
        CloseWindow(w);
    }
    return(NULL);
}

void closeWindow(struct Window *w)
{
    disposeWindowData((struct windowData *)w->UserData);
    CloseWindow(w);
}

struct windowData *newWindowData(struct Window *w)
{
    struct windowData *wd;

    if (wd = AllocMem(sizeof(*wd), MEMF_PUBLIC|MEMF_CLEAR))
    {
        w->UserData = (APTR)wd;
        wd->win = w;
        return(wd);
    }
    return(NULL);
}

void disposeWindowData(struct windowData *wd)
{
    struct Gadget *gad, *next;

    gad = wd->gad;
    while (gad)
    {
        next = gad->NextGadget;
        disposeGad(gad);
        gad = next;
    }
    FreeMem(wd, sizeof(*wd));
}

struct Gadget *newGadget(struct windowData *wd)
{
    struct Gadget *gad;

    if (gad = AllocMem(sizeof(*gad), MEMF_PUBLIC|MEMF_CLEAR))
    {
        gad->GadgetType = GTYP_BOOLGADGET;
        gad->Activation = GACT_IMMEDIATE|GACT_RELVERIFY|GACT_FOLLOWMOUSE;
        gad->Flags      = GFLG_GADGHNONE;

        gad->NextGadget = wd->gad;
        wd->gad = gad;
        return(gad);
    }
    return(NULL);
}

void disposeGadget(struct Gadget *gad)
{
    FreeMem(gad, sizeof(*gad));
}

void handleWindow(struct IntuiMessage *msg)
{
    struct Window *win = msg->IDCMPWindow;
    struct windowData *wd = (struct windowData *)win->UserData;

    switch (msg->Class)
    {
        case IDCMP_GADGETDOWN:
            struct Gadget *gad = (struct Gadget *)msg->IAddress;
            struct gadgetData *gd = (struct gadgetData *)gad->UserData;
            wd->active = gad;
            if (gd->onGadgetDown)
            {
                gd->onGadgetDown(gd, msg->MouseX, msg->MouseY);
            }
            break;

        case IDCMP_GADGETUP:
            struct Gadget *gad = (struct Gadget *)msg->IAddress;
            struct gadgetData *gd = (struct gadgetData *)gad->UserData;
            wd->active = NULL;
            if (gd->onGadgetUp)
            {
                gd->onGadgetUp(gd, msg->MouseX, msg->MouseY);
            }
            break;

        case IDCMP_MOUSEMOVE:
            struct Gadget *gad = wd->active;
            if (gad)
            {
                struct gadgetData *gd = (struct gadgetData *)gad->UserData;
                if (gd->onGadgetDown)
                {
                    gd->onMouseMove(gd, msg->MouseX, msg->MouseY);
                }
            }
            break;

        case IDCMP_RAWKEY:
            if (wd->onRawKey)
            {
                wd->onRawKey(wd, msg->Code, msg->Qualifier);
            }
            break;
    }
}
