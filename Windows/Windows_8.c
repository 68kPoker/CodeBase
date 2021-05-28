
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>

#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

#include "Windows.h"

BOOL initWindow(struct Window *w, WD wd)
{
    if (wd->group = NewObject(NULL, "groupgclass",
        TAG_DONE))
    {
        if (wd->frame = NewObject(NULL, "frameiclass",
            TAG_DONE))
        {
            if (wd->dri = GetScreenDrawInfo(w->WScreen))
            {
                AddGadget(w, (struct Gadget *)wd->group, -1);
                wd->w = w;
                return(TRUE);
            }
            DisposeObject(wd->frame);
        }
        DisposeObject(wd->group);
    }
    return(FALSE);
}

BOOL addItem(WD wd, STRPTR name, WORD gid, WORD left, WORD top, WORD width, WORD height)
{
    Object *gad;

    if (gad = NewObject(NULL, "frbuttonclass",
        GA_Left,    left,
        GA_Top,     top,
        GA_Image,   wd->frame,
        GA_ID,      gid,
        GA_Text,    name,
        GA_DrawInfo,    wd->dri,
        TAG_DONE))
    {
        SetGadgetAttrs((struct Gadget *)gad, NULL, NULL,
            GA_Width,   width,
            GA_Height,  height,
            TAG_DONE);

        DoMethod(wd->group, OM_ADDMEMBER, gad);
        return(TRUE);
    }
    return(FALSE);
}

void closeWindow(WD wd)
{
    RemoveGadget(wd->w, (struct Gadget *)wd->group);
    FreeScreenDrawInfo(wd->w->WScreen, wd->dri);
    DisposeObject(wd->frame);
    DisposeObject(wd->group);
}
