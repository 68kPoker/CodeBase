
/* Obsîuga joysticka */

/* $Log:	Joy.c,v $
 * Revision 1.1  12/.1/.2  .1:.5:.0  Robert
 * Initial revision
 *  */

#include <devices/gameport.h>
#include <devices/inputevent.h>

#include <clib/exec_protos.h>

#include "Joy.h"

BOOL setGPCT(IOStd io)
{
    BYTE gpct;
    BOOL ok = FALSE;

    Forbid();

    io->io_Command  = GPD_ASKCTYPE;
    io->io_Data     = (APTR)&gpct;
    io->io_Length   = 1;
    io->io_Flags    = IOF_QUICK;
    DoIO((IO)io);

    if (gpct == GPCT_NOCONTROLLER)
    {
        ok = TRUE;
        gpct = GPCT_ABSJOYSTICK;

        io->io_Command  = GPD_SETCTYPE;
        io->io_Data     = (APTR)&gpct;
        io->io_Length   = 1;
        io->io_Flags    = IOF_QUICK;
        DoIO((IO)io);
    }

    Permit();
    return(ok);
}

void setTrigger(IOStd io)
{
    struct GamePortTrigger gpt;

    gpt.gpt_XDelta = gpt.gpt_YDelta = 1;
    gpt.gpt_Timeout = TIMEOUT;
    gpt.gpt_Keys = GPTF_UPKEYS | GPTF_DOWNKEYS;

    io->io_Command  = GPD_SETTRIGGER;
    io->io_Data     = (APTR)&gpt;
    io->io_Length   = sizeof(gpt);
    io->io_Flags    = IOF_QUICK;
    DoIO((IO)io);
}

void clearIO(IOStd io)
{
    io->io_Command  = CMD_CLEAR;
    io->io_Data     = NULL;
    io->io_Length   = 0;
    io->io_Flags    = IOF_QUICK;
    DoIO((IO)io);
}

void readEvent(IOStd io, struct InputEvent *ie)
{
    io->io_Command  = GPD_READEVENT;
    io->io_Data     = (APTR)ie;
    io->io_Length   = sizeof(*ie);
    io->io_Flags    = 0;
    SendIO((IO)io);
}

void clearGPCT(IOStd io)
{
    BYTE gpct = GPCT_NOCONTROLLER;

    io->io_Command  = GPD_SETCTYPE;
    io->io_Data     = (APTR)&gpct;
    io->io_Length   = 1;
    io->io_Flags    = IOF_QUICK;
    DoIO((IO)io);
}

IOStd openJoy(struct InputEvent *ie)
{
    struct MsgPort *mp;
    IOStd io;

    if (mp = CreateMsgPort())
    {
        if (io = CreateIORequest(mp, sizeof(*io)))
        {
            if (OpenDevice("gameport.device", 1, (IO)io, 0) == 0)
            {
                /* Check and set controller */
                if (setGPCT(io))
                {
                    setTrigger(io);
                    clearIO(io);
                    readEvent(io, ie);
                    return(io);
                }
                CloseDevice((IO)io);
            }
            DeleteIORequest((IO)io);
        }
        DeleteMsgPort(mp);
    }
    return(NULL);
}

void closeJoy(IOStd io)
{
    struct MsgPort *mp = io->io_Message.mn_ReplyPort;

    if (!CheckIO((IO)io))
    {
        AbortIO((IO)io);
    }
    WaitIO((IO)io);
    clearGPCT(io);
    CloseDevice((IO)io);
    DeleteIORequest((IO)io);
    DeleteMsgPort(mp);
}
