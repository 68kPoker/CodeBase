
#include "Gadgets.h"

#include <exec/memory.h>
#include <clib/exec_protos.h>

/* Create empty gadget */

struct gadgetInfo *newGadget(void)
{
    struct gadgetInfo *gi;

    if (gi = AllocMem(sizeof(*gi), MEMF_PUBLIC|MEMF_CLEAR))
    {
        gi->gad.Flags = GFLG_GADGHCOMP;
        gi->gad.Activation = GACT_RELVERIFY;

        gi->gad.UserData = (APTR)gi;
        return(gi);
    }
    return(NULL);
}

/* Set gadget attributes */

void setGadgetBox(struct gadgetInfo *gi, WORD left, WORD top, WORD width, WORD height)
{
    gi->gad.LeftEdge = left;
    gi->gad.TopEdge  = top;
    gi->gad.Width    = width;
    gi->gad.Height   = height;
}

void setGadgetID(struct gadgetInfo *gi, WORD gid)
{
    gi->gad.GadgetID = gid;
}

void setGadgetHandle(struct gadgetInfo *gi, void (*handle)(struct gadgetInfo *gi))
{
    gi->handleUp = handle;
}

void linkGadgets(struct MinList *list)
{
    struct MinNode *mn;
    struct gadgetInfo *prev = NULL;

    for (mn = list->mlh_Head; mn->mln_Succ != NULL; mn = mn->mln_Succ)
    {
        struct gadgetInfo *gi = (struct gadgetInfo *)mn;

        if (prev)
        {
            gi->gad.NextGadget = &prev->gad;
        }
        prev = gi;
    }
}

void disposeGadget(struct gadgetInfo *gi)
{
    FreeMem(gi, sizeof(*gi));
}
