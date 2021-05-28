
#include <stdio.h>
#include "debug.h"

#include <devices/gameport.h>
#include <devices/inputevent.h>

#include <clib/exec_protos.h>

struct IORequest *openDevice(STRPTR name, UBYTE unit, WORD size, ULONG flags)
{
    struct MsgPort *mp;

    if (mp = CreateMsgPort())
    {
        struct IORequest *io;
        if (io = CreateIORequest(mp, size))
        {
            if (!OpenDevice(name, unit, io, flags))
            {
                DD(bug("%s opened.\n", name));

                return io;
            }
            DeleteIORequest(io);
        }
        DeleteMsgPort(mp);
    }
    return NULL;
}

void closeDevice(struct IORequest *io)
{
    struct MsgPort *mp = io->io_Message.mn_ReplyPort;

    CloseDevice(io);
    DeleteIORequest(io);
    DeleteMsgPort(mp);
}

void waitIO(struct IORequest *io)
{
    if (!CheckIO(io))
        AbortIO(io);

    WaitIO(io);
}

BOOL checkController(struct IOStdReq *io, UBYTE newCon)
{
    UBYTE oldCon;
    BOOL notInUse = FALSE;

    Forbid();

    io->io_Command  = GPD_ASKCTYPE;
    io->io_Data     = (APTR)&oldCon;
    io->io_Length   = sizeof(oldCon);
    io->io_Flags    = IOF_QUICK;

    DoIO((struct IORequest *)io);

    if (oldCon == GPCT_NOCONTROLLER)
    {
        notInUse = TRUE;

        io->io_Command  = GPD_SETCTYPE;
        io->io_Data     = (APTR)&newCon;
        io->io_Length   = sizeof(newCon);
        io->io_Flags    = IOF_QUICK;

        DoIO((struct IORequest *)io);
    }

    Permit();

    return notInUse;
}

void freeController(struct IOStdReq *io)
{
    UBYTE con = GPCT_NOCONTROLLER;

    io->io_Command  = GPD_SETCTYPE;
    io->io_Data     = (APTR)&con;
    io->io_Length   = sizeof(con);
    io->io_Flags    = IOF_QUICK;

    DoIO((struct IORequest *)io);
}

void setTrigger(struct IOStdReq *io, struct GamePortTrigger *gpt)
{
    io->io_Command  = GPD_SETTRIGGER;
    io->io_Data     = (APTR)gpt;
    io->io_Length   = sizeof(*gpt);
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

void readEvent(struct IOStdReq *io, struct InputEvent *ie)
{
    io->io_Command  = GPD_READEVENT;
    io->io_Data     = (APTR)ie;
    io->io_Length   = sizeof(*ie);
    io->io_Flags    = 0;

    SendIO((struct IORequest *)io);
}

struct IOStdReq *openGameport(UBYTE unit, WORD timeOut, struct InputEvent *ie)
{
    struct IOStdReq *io;

    if (io = (struct IOStdReq *)openDevice("gameport.device", unit, sizeof(*io), 0))
    {
        if (checkController(io, GPCT_ABSJOYSTICK))
        {
            struct GamePortTrigger gpt;
            gpt.gpt_Keys = GPTF_UPKEYS|GPTF_DOWNKEYS;
            gpt.gpt_XDelta = gpt.gpt_YDelta = 1;
            gpt.gpt_Timeout = timeOut;

            setTrigger(io, &gpt);
            clearIO(io);
            readEvent(io, ie);

            return io;
        }
        closeDevice((struct IORequest *)io);
    }
    return NULL;
}

void closeGameport(struct IOStdReq *io)
{
    waitIO((struct IORequest *)io);
    freeController(io);
    closeDevice((struct IORequest *)io);
}
