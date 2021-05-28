
/* $Log$ */

#include <graphics/view.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <graphics/gfxmacros.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>

#include <clib/exec_protos.h>

#include "Copper.h"

__far extern struct Custom custom;

BOOL addCopperInt(struct Interrupt *is, struct copperData *cd, struct ViewPort *vp)
{
    is->is_Code = myCopper;
    is->is_Data = cd;
    is->is_Node.ln_Pri = 0;
    is->is_Node.ln_Name = "GearWorks";

    if ((cd->signal = AllocSignal(-1)) != -1)
    {
        cd->vp = vp;
        cd->task = FindTask(NULL);
        AddIntServer(INTB_COPER, is);
        return(TRUE);
    }
    return(FALSE);
}

VOID remCopperInt(struct Interrupt *is)
{
    struct copperData *cd = (struct copperData *)is->is_Data;

    RemIntServer(INTB_COPER, is);

    FreeSignal(cd->signal);
}

BOOL addCopperList(struct ViewPort *vp)
{
    struct UCopList *ucl;

    if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
    {
        CINIT(ucl, 3);
        CWAIT(ucl, COPLINE, 0);
        CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
        CEND(ucl);

        Forbid();
        vp->UCopIns = ucl;
        Permit();

        RethinkDisplay();
        return(TRUE);
    }
    return(FALSE);
}
