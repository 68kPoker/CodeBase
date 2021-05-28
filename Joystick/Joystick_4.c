
#include <devices/gameport.h>
#include <clib/exec_protos.h>

#include "CInput.h"

#define TIMEOUT 100

void clearCType( struct IOStdReq *io )
{
    UBYTE con = GPCT_NOCONTROLLER;

    io->io_Command  = GPD_SETCTYPE;
    io->io_Data     = ( APTR ) &con;
    io->io_Length   = 1;
    io->io_Flags    = IOF_QUICK;
    DoIO( ( struct IORequest * ) io );
}

BOOL checkCType( struct IOStdReq *io )
{
    BOOL ok = FALSE;
    UBYTE con;

    Forbid();

    io->io_Command  = GPD_ASKCTYPE;
    io->io_Data     = ( APTR ) &con;
    io->io_Length   = 1;
    io->io_Flags    = IOF_QUICK;
    DoIO( ( struct IORequest * ) io );

    if( con == GPCT_NOCONTROLLER ) {
        ok = TRUE;
        con = GPCT_ABSJOYSTICK;

        io->io_Command  = GPD_SETCTYPE;
        io->io_Data     = ( APTR ) &con;
        io->io_Length   = 1;
        io->io_Flags    = IOF_QUICK;
        DoIO( ( struct IORequest * ) io );
    }

    Permit();

    return( ok );
}

void setTrigger( struct IOStdReq *io )
{
    struct GamePortTrigger gpt;

    gpt.gpt_Keys = GPTF_UPKEYS | GPTF_DOWNKEYS;
    gpt.gpt_XDelta = 1;
    gpt.gpt_YDelta = 1;
    gpt.gpt_Timeout = TIMEOUT;

    io->io_Command  = GPD_SETTRIGGER;
    io->io_Data     = ( APTR ) &gpt;
    io->io_Length   = sizeof( gpt );
    io->io_Flags    = IOF_QUICK;
    DoIO( ( struct IORequest * ) io );
}

void clearIO( struct IOStdReq *io )
{
    io->io_Command  = CMD_CLEAR;
    io->io_Data     = ( APTR ) NULL;
    io->io_Length   = 0;
    io->io_Flags    = IOF_QUICK;
    DoIO( ( struct IORequest * ) io );
}

void readEvent( CController *con )
{
    struct IOStdReq *io = con->io;

    io->io_Command  = GPD_READEVENT;
    io->io_Data     = ( APTR ) &con->ie;
    io->io_Length   = sizeof( struct InputEvent );
    io->io_Flags    = 0;
    SendIO( ( struct IORequest * ) io );
}

BOOL openController( CInput *in, CController *con, CInputHandler handler )
{
    struct MsgPort *mp;
    struct IORequest *io;

    if( mp = CreateMsgPort() ) {
        if( io = CreateIORequest( mp, sizeof( struct IOStdReq ) ) ) {
            if( OpenDevice( "gameport.device", 1, io, 0 ) == 0 ) {
                con->io = ( struct IOStdReq * ) io;

                if( checkCType( con->io ) ) {
                    setTrigger( con->io );
                    clearIO( con->io );
                    readEvent( con );

                    initInput( in, 1L << mp->mp_SigBit, handler, con );
                    return( TRUE );
                }
                CloseDevice( io );
            }
            DeleteIORequest( io );
        }
        DeleteMsgPort( mp );
    }
    return( FALSE );
}

void closeController( CController *con )
{
    struct IORequest *io = ( struct IORequest * ) con->io;
    struct MsgPort *mp = io->io_Message.mn_ReplyPort;

    if( !CheckIO( io ) )
        AbortIO( io );
    WaitIO( io );

    clearCType( con->io );
    CloseDevice( io );
    DeleteIORequest( io );
    DeleteMsgPort( mp );
}
