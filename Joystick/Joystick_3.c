
#include <exec/memory.h>
#include <devices/gameport.h>
#include <clib/exec_protos.h>

#include "Joystick.h"
#include "Joystick_protos.h"

BOOL checkController(struct IOStdReq *io)
{
    UBYTE c;
    BOOL ok = FALSE;

    Forbid();

    io->io_Command  = GPD_ASKCTYPE;
    io->io_Data     = &c;
    io->io_Length   = 1;
    io->io_Flags    = IOF_QUICK;
    DoIO((struct IORequest *)io);

    if (c == GPCT_NOCONTROLLER)
    {
        ok = TRUE;

        c = GPCT_ABSJOYSTICK;

        io->io_Command  = GPD_SETCTYPE;
        io->io_Data     = &c;
        io->io_Length   = 1;
        io->io_Flags    = IOF_QUICK;
        DoIO((struct IORequest *)io);
    }

    Permit();
    return(ok);
}

void resetController(struct IOStdReq *io)
{
    UBYTE c = GPCT_NOCONTROLLER;

    io->io_Command  = GPD_SETCTYPE;
    io->io_Data     = &c;
    io->io_Length   = 1;
    io->io_Flags    = IOF_QUICK;
    DoIO((struct IORequest *)io);
}

void setTrigger(struct IOStdReq *io)
{
    struct GamePortTrigger gpt;

    gpt.gpt_Keys = GPTF_UPKEYS|GPTF_DOWNKEYS;
    gpt.gpt_XDelta = 1;
    gpt.gpt_YDelta = 1;
    gpt.gpt_Timeout = 10;

    io->io_Command  = GPD_SETTRIGGER;
    io->io_Data     = &gpt;
    io->io_Length   = sizeof(gpt);
    io->io_Flags    = IOF_QUICK;
    DoIO((struct IORequest *)io);
}

void clearIO(struct IOStdReq *io)
{
    io->io_Command  = CMD_CLEAR;
    io->io_Data     = NULL;
    io->io_Length   = 0;
    io->io_Flags    = IOF_QUICK;
    DoIO((struct IORequest *)io);
}

void readEvent(JOY *joy)
{
    struct IOStdReq *io = joy->io;

    io->io_Command  = GPD_READEVENT;
    io->io_Data     = &joy->ie;
    io->io_Length   = sizeof(joy->ie);
    io->io_Flags    = 0;
    SendIO((struct IORequest *)io);
}

JOY *openJoystick()
{
    JOY *joy;

    if (joy = AllocMem(sizeof(*joy), MEMF_PUBLIC))
    {
        struct MsgPort *mp;

        if (mp = CreateMsgPort())
        {
            struct IOStdReq *io;

            if (joy->io = io = CreateIORequest(mp, sizeof(*io)))
            {
                if (!OpenDevice("gameport.device", 1, (struct IORequest *)io, 0))
                {
                    if (checkController(io))
                    {
                        setTrigger(io);
                        clearIO(io);
                        readEvent(joy);
                        return(joy);
                    }
                    CloseDevice((struct IORequest *)io);
                }
                DeleteIORequest(io);
            }
            DeleteMsgPort(mp);
        }
        FreeMem(joy, sizeof(*joy));
    }
    return(NULL);
}

void closeJoystick(JOY *joy)
{
    struct MsgPort *mp = joy->io->io_Message.mn_ReplyPort;
    struct IOStdReq *io = joy->io;

    if (!CheckIO((struct IORequest *)io))
    {
        AbortIO((struct IORequest *)io);
    }
    WaitIO((struct IORequest *)io);

    resetController(io);
    CloseDevice((struct IORequest *)io);
    DeleteIORequest((struct IORequest *)io);
    DeleteMsgPort(mp);

    FreeMem(joy, sizeof(*joy));
}
