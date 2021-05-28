
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>
#include <intuition/screens.h>
#include <graphics/gfxmacros.h>

#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>

#include "Screen.h"

__far extern struct Custom custom;
extern void myCopper(void);

BOOL addUserCopList(struct Screen *s)
{
    struct UCopList *ucl;

    if (ucl = AllocMem(sizeof(*ucl), MEMF_PUBLIC|MEMF_CLEAR))
    {
        CINIT(ucl, 3);
        CWAIT(ucl, 0, 0);
        CMOVE(ucl, custom.intreq, INTF_SETCLR|INTF_COPER);
        CEND(ucl);

        Forbid();
        s->ViewPort.UCopIns = ucl;
        Permit();

        RethinkDisplay();
        return(TRUE);
    }
    return(FALSE);
}

BOOL addCopperInterrupt(struct ViewPort *vp, struct Interrupt *is, struct copperInfo *ci)
{
    if ((ci->signal = AllocSignal(-1)) != -1)
    {
        ci->vp = vp;
        ci->task = FindTask(NULL);

        is->is_Code = myCopper;
        is->is_Data = (APTR)ci;
        is->is_Node.ln_Pri = 0;

        AddIntServer(INTB_COPER, is);
        return(TRUE);
    }
    return(FALSE);
}

void remCopperInterrupt(struct Interrupt *is)
{
    struct copperInfo *ci = (struct copperInfo *)is->is_Data;

    RemIntServer(INTB_COPER, is);
    FreeSignal(ci->signal);
}
