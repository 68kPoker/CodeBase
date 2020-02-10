
/*
**  (C)2018-2020 Robert Szacki Software House
**
**  » Magazyn «
**
**  $Id: Joystick.c,v 1.1 12/.0/.0 .0:.3:.3 Robert Exp $
*/

#include <devices/gameport.h>
#include <devices/inputevent.h>

#include <clib/exec_protos.h>

#include "Joystick.h"

struct IOStdReq* openGameport(struct InputEvent* ie)
{
    struct MsgPort* mp;

    if (mp = CreateMsgPort())
    {
        struct IOStdReq* io;

        if (io = (struct IOStdReq* )CreateIORequest(mp, sizeof(*io)))
        {
            if (OpenDevice("gameport.device", 1, (struct IORequest* )io, 0) == 0)
            {
                if (setController(io))
                {
                    setTrigger(io);
                    clearIO(io);
                    readEvent(io, ie);
                    return(io);
                }
                CloseDevice((struct IORequest* )io);
            }
            DeleteIORequest((struct IORequest* )io);
        }
        DeleteMsgPort(mp);
    }
    return(NULL);
}

void closeGameport(struct IOStdReq* io)
{
    struct MsgPort* mp = io->io_Message.mn_ReplyPort;
    if (CheckIO((struct IORequest* )io))
    {
        AbortIO((struct IORequest* )io);
    }
    WaitIO((struct IORequest* )io);
    resetController(io);
    CloseDevice((struct IORequest* )io);
    DeleteIORequest((struct IORequest* )io);
    DeleteMsgPort(mp);
}

BOOL setController(struct IOStdReq* io)
{
    BOOL done = FALSE;
    UBYTE ctrl;

    Forbid();
    io->io_Command = GPD_ASKCTYPE;
    io->io_Data    = &ctrl;
    io->io_Length  = 1;
    io->io_Flags   = IOF_QUICK;
    DoIO((struct IORequest* )io);

    if (ctrl == GPCT_NOCONTROLLER)
    {
        ctrl = GPCT_ABSJOYSTICK;
        done = TRUE;
        io->io_Command = GPD_SETCTYPE;
        io->io_Data    = &ctrl;
        io->io_Length  = 1;
        io->io_Flags   = IOF_QUICK;
        DoIO((struct IORequest* )io);
    }

    Permit();
    return(done);
}

void resetController(struct IOStdReq* io)
{
    UBYTE ctrl = GPCT_NOCONTROLLER;

    io->io_Command = GPD_SETCTYPE;
    io->io_Data    = &ctrl;
    io->io_Length  = 1;
    io->io_Flags   = IOF_QUICK;
    DoIO((struct IORequest* )io);
}

void setTrigger(struct IOStdReq* io)
{
    struct GamePortTrigger gpt;

    gpt.gpt_Keys    = GPTF_UPKEYS|GPTF_DOWNKEYS;
    gpt.gpt_XDelta  = 1;
    gpt.gpt_YDelta  = 1;
    gpt.gpt_Timeout = TIMEOUT;

    io->io_Command = GPD_SETTRIGGER;
    io->io_Data    = &gpt;
    io->io_Length  = sizeof(gpt);
    io->io_Flags   = IOF_QUICK;
    DoIO((struct IORequest* )io);
}

void clearIO(struct IOStdReq* io)
{
    io->io_Command = CMD_CLEAR;
    io->io_Data    = NULL;
    io->io_Length  = 0;
    io->io_Flags   = IOF_QUICK;
    DoIO((struct IORequest* )io);
}

void readEvent(struct IOStdReq* io, struct InputEvent* ie)
{
    io->io_Command = GPD_READEVENT;
    io->io_Data    = ie;
    io->io_Length  = sizeof(*ie);
    io->io_Flags   = 0;
    SendIO((struct IORequest* )io);
}

/* EOF */
