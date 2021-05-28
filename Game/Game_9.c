
#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <clib/exec_protos.h>
#include <clib/gadtools_protos.h>

#define ESC_KEY (0x45)

enum
{
    SRC_IDCMP, /* Window */
    SRC_SAFE,  /* Safe to write */
    SRC_DISP,  /* Safe to change */
    SRC_COUNT
};

ULONG signals[SRC_COUNT];

LONG (*handle[SRC_COUNT])() =
{
    handleIDCMP,
    handleSafe,
    handleDisp
};

struct MsgPort *userPort;

/* Wait for and handle signals */
LONG loop(ULONG total, LONG (*handle)(ULONG result))
{
    LONG done;

    do
    {
        /* Wait for signal */
        ULONG result = Wait(total);
    }
    /* Use handler */
    while (!(done = handle(result)));
    return(0);
}

/* Standard handler */
LONG handle(ULONG result)
{
    WORD i;

    for (i = 0; i < SRC_COUNT; i++)
    {
        if (result & signals[i])
        {
            /* Handle this signal */
            if (handle[i]())
                return(TRUE);
        }
    }
    return(FALSE);
}

LONG handleIDCMP()
{
    struct IntuiMessage *msg;

    while (msg = GT_GetIMsg(userPort))
    {
        struct Window *win = msg->IDCMPWindow;
        struct windowData *data = (struct windowData *)win->UserData;

        ULONG class = msg->Class;
        UWORD code  = msg->Code;
        WORD  mx    = msg->MouseX;
        WORD  my    = msg->MouseY;
        APTR  iaddr = msg->IAddress;

        GT_ReplyIMsg(msg);

        switch (class)
        {
            case IDCMP_RAWKEY:
                if (code == ESC_KEY)
                    return(TRUE);
                break;
        }
    }
    return(FALSE);
}
