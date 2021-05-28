
/* Obsîuga joysticka */

/* Potrzebna funkcjonalnoôê: */

/* - Odczyt stanu joysticka X, Y, FIRE */

#include <devices/gameport.h>
#include <devices/inputevent.h>
#include <clib/exec_protos.h>

#include "Joystick.h"

void readevent(struct IOStdReq *io, struct InputEvent *ie)
{
    io->io_Command  = GPD_READEVENT;
    io->io_Data     = (APTR)ie;
    io->io_Length   = sizeof(*ie);
    io->io_Flags    = 0;
    SendIO((struct IORequest *)io);
}

BOOL getevent(struct IOStdReq *io)
{
    if (GetMsg(io->io_Message.mn_ReplyPort))
    {
        return(TRUE);
    }
    return(FALSE);
}

BOOL getcontroller(struct IOStdReq *io)
{
    BOOL ok = FALSE;
    UBYTE con;

    Forbid();

    io->io_Command  = GPD_ASKCTYPE;
    io->io_Data     = (APTR)&con;
    io->io_Length   = sizeof(con);
    io->io_Flags    = IOF_QUICK;
    DoIO((struct IORequest *)io);

    if (con == GPCT_NOCONTROLLER)
    {
        ok = TRUE;
        con = GPCT_ABSJOYSTICK;

        io->io_Command  = GPD_SETCTYPE;
        io->io_Data     = (APTR)&con;
        io->io_Length   = sizeof(con);
        io->io_Flags    = IOF_QUICK;
        DoIO((struct IORequest *)io);
    }

    Permit();
    return(ok);
}

void freecontroller(struct IOStdReq *io)
{
    UBYTE con = GPCT_NOCONTROLLER;

    io->io_Command  = GPD_SETCTYPE;
    io->io_Data     = (APTR)&con;
    io->io_Length   = sizeof(con);
    io->io_Flags    = IOF_QUICK;
    DoIO((struct IORequest *)io);
}

void settrigger(struct IOStdReq *io)
{
    struct GamePortTrigger gpt;

    gpt.gpt_Keys = GPTF_UPKEYS|GPTF_DOWNKEYS;
    gpt.gpt_XDelta = 1;
    gpt.gpt_YDelta = 1;
    gpt.gpt_Timeout = TIMEOUT;

    io->io_Command  = GPD_SETTRIGGER;
    io->io_Data     = (APTR)&gpt;
    io->io_Length   = sizeof(gpt);
    io->io_Flags    = IOF_QUICK;
    DoIO((struct IORequest *)io);
}

void cleario(struct IOStdReq *io)
{
    io->io_Command  = CMD_CLEAR;
    io->io_Data     = (APTR)NULL;
    io->io_Length   = 0;
    io->io_Flags    = IOF_QUICK;
    DoIO((struct IORequest *)io);
}

struct IOStdReq *openjoy(struct InputEvent *ie)
{
    struct MsgPort *mp;

    if (mp = CreateMsgPort())
    {
        struct IOStdReq *io;

        if (io = (struct IOStdReq *)CreateIORequest(mp, sizeof(*io)))
        {
            if (OpenDevice("gameport.device", 1, (struct IORequest *)io, 0) == 0)
            {
                if (getcontroller(io))
                {
                    settrigger(io);
                    cleario(io);
                    readevent(io, ie);
                    return(io);
                }
                CloseDevice((struct IORequest *)io);
            }
            DeleteIORequest((struct IORequest *)io);
        }
        DeleteMsgPort(mp);
    }
    return(NULL);
}

void closejoy(struct IOStdReq *io)
{
    struct MsgPort *mp = io->io_Message.mn_ReplyPort;

    if (CheckIO((struct IORequest *)io) == NULL)
        AbortIO((struct IORequest *)io);

    WaitIO((struct IORequest *)io);

    freecontroller(io);
    CloseDevice((struct IORequest *)io);
    DeleteIORequest((struct IORequest *)io);
    DeleteMsgPort(mp);
}
