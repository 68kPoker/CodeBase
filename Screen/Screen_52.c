
/* $Id$ */

#include "Screen.h"

#include <intuition/screens.h>
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <graphics/gfxmacros.h>
#include <hardware/intbits.h>
#include <hardware/custom.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

far extern struct Custom custom;

extern void copperServer();

struct screenData *screenOpen( UBYTE colorDepth )
{
    struct screenData *sd;
    struct Rectangle dclip =
    {
        0, 0,
        319, 255
    };
    ULONG displayID = LORES_KEY;
    static UBYTE title[] = "Gear Works";
    const BYTE copPri = 0;
    const WORD copLength = 3;
    const WORD copLine = 0;

    if( sd = AllocMem( sizeof( *sd ), MEMF_PUBLIC | MEMF_CLEAR ) )
    {
        if( sd->screen = OpenScreenTags( NULL,
            SA_DClip,       &dclip,
            SA_Depth,       colorDepth,
            SA_DisplayID,   displayID,
            SA_Title,       title,
            SA_ShowTitle,   FALSE,
            SA_Exclusive,   TRUE,
            SA_Quiet,       TRUE,
            SA_Draggable,   FALSE,
            SA_BackFill,    LAYERS_NOBACKFILL,
            SA_Interleaved, TRUE,
            TAG_DONE ) )
        {
            sd->screen->UserData = ( APTR ) sd;
            if( sd->buffers[ 0 ] = AllocScreenBuffer( sd->screen, NULL, SB_SCREEN_BITMAP ) )
            {
                if( sd->buffers[ 1 ] = AllocScreenBuffer( sd->screen, NULL, 0 ) )
                {
                    if( sd->safePort = CreateMsgPort() )
                    {
                        sd->buffers[ 0 ]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sd->safePort;
                        sd->buffers[ 1 ]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = sd->safePort;
                        sd->safeToDraw = TRUE;
                        sd->toggleFrame = 1;

                        if( sd->is = AllocMem( sizeof( *sd->is ), MEMF_PUBLIC | MEMF_CLEAR ) )
                        {
                            sd->is->is_Code = copperServer;
                            sd->is->is_Data = ( APTR ) &sd->cop;
                            sd->is->is_Node.ln_Pri = copPri;
                            sd->is->is_Node.ln_Name = title;

                            if( ( sd->cop.signal = AllocSignal( -1 ) ) != -1 )
                            {
                                struct UCopList *ucl;

                                sd->cop.task = FindTask( NULL );
                                sd->cop.vp = &sd->screen->ViewPort;

                                if( ucl = AllocMem( sizeof( *ucl ), MEMF_PUBLIC | MEMF_CLEAR ) )
                                {
                                    CINIT( ucl, copLength );
                                    CWAIT( ucl, copLine, 0 );
                                    CMOVE( ucl, custom.intreq, INTF_SETCLR | INTF_COPER );
                                    CEND( ucl );

                                    Forbid();
                                    sd->screen->ViewPort.UCopIns = ucl;
                                    Permit();

                                    RethinkDisplay();

                                    if ( sd->userPort = CreateMsgPort() )
                                    {
                                        AddIntServer( INTB_COPER, sd->is );

                                        return( sd );
                                    }
                                }
                                FreeSignal( sd->cop.signal );
                            }
                            FreeMem( sd->is, sizeof( *sd->is ) );
                        }
                        DeleteMsgPort( sd->safePort );
                    }
                    FreeScreenBuffer( sd->screen, sd->buffers[ 1 ] );
                }
                FreeScreenBuffer( sd->screen, sd->buffers[ 0 ] );
            }
            CloseScreen( sd->screen );
        }
        FreeMem( sd, sizeof( *sd ) );
    }
    return( NULL );
}

VOID screenClose( struct screenData *sd )
{
    RemIntServer( INTB_COPER, sd->is );
    DeleteMsgPort( sd->userPort );
    FreeSignal( sd->cop.signal );
    FreeMem( sd->is, sizeof( *sd->is ) );

    if( !sd->safeToDraw )
    {
        while( !GetMsg( sd->safePort ) )
        {
            WaitPort( sd->safePort );
        }
    }
    DeleteMsgPort( sd->safePort );
    FreeScreenBuffer( sd->screen, sd->buffers[ 1 ] );
    FreeScreenBuffer( sd->screen, sd->buffers[ 0 ] );
    CloseScreen( sd->screen );
    FreeMem( sd, sizeof( *sd ) );
}
