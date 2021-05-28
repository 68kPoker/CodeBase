
/* $Id$ */

/* Game specific functions */

#include <stdio.h>
#include "System.h"
#include "Screen.h"
#include "Windows.h"
#include "IFF.h"
#include "debug.h"

#include <workbench/startup.h>
#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/layers_protos.h>

#define DEPTH 5

LONG backHandler( struct windowData *wd, struct windowMsg *msg )
{
    struct screenData *sd = ( struct screenData * ) wd->window->WScreen->UserData;
    static WORD counter = 0;
    struct RastPort *rp = wd->window->RPort;
    static UBYTE text[5] = "    ";
    struct IntuiMessage *imsg;
    UBYTE toggleFrame = sd->toggleFrame;
    struct Rectangle rect;
    WORD x, y;
    static WORD prevx[2] = { 0 }, prevy[2] = { 0 };

    DD(bug("%d (%d)\n", msg->kind, counter));

    switch( msg->kind )
    {
        case WINDOW_MSG_INIT:
            counter = 0;

                rect.MinX = ( counter % 20 ) << 4;
                rect.MaxX = ( ( counter % 20 ) << 4 ) + 15;
                rect.MinY = ( counter / 20 ) << 4;
                rect.MaxY = ( ( counter / 20 ) << 4 ) + 15;

            OrRectRegion( wd->update[ toggleFrame ], &rect );
            wd->flags = 0x3;
            break;

        case WINDOW_MSG_USER:
            imsg = msg->imsg;
            if( imsg->Class == IDCMP_RAWKEY )
            {
                if( imsg->Code == ESC_KEY )
                {
                    wd->close = TRUE;
                }
            }
            break;

        case WINDOW_MSG_DRAW:

            x = ( counter % 20 ) << 4;
            y = ( counter / 20 ) << 4;

            BltBitMapRastPort( sd->gfx, 0, 0, rp, x, y, 16, 16, 0xc0 );
            BltBitMapRastPort( sd->gfx, 0, 0, rp, prevx[toggleFrame^1], prevy[toggleFrame^1], 16, 16, 0xc0 );

            Move( rp, 0, rp->Font->tf_Baseline );
            SetABPenDrMd( rp, 30, 0, JAM2 );
            Text( rp, text, 4 );

            break;

        case WINDOW_MSG_ANIMATE:

            x = ( counter % 20 ) << 4;
            y = ( counter / 20 ) << 4;

            if (counter < 320)
            {
            prevx[toggleFrame] = x;
            prevy[toggleFrame] = y;

                counter++;

                rect.MinX = ( counter % 20 ) << 4;
                rect.MaxX = ( ( counter % 20 ) << 4 ) + 15;
                rect.MinY = ( counter / 20 ) << 4;
                rect.MaxY = ( ( counter / 20 ) << 4 ) + 15;

                OrRectRegion( wd->update[ toggleFrame ], &rect );
                wd->flags = 0x3;

                rect.MinX = 0;
                rect.MinY = 0;
                rect.MaxX = 63;
                rect.MaxY = 15;
                OrRectRegion( wd->update[ toggleFrame ], &rect );

                sprintf( text, "%4d", counter );

            }

            break;
    }
    return( 0 );
}

LONG gameLoop( struct screenData *sd )
{
    struct windowData *backwd = sd->backwd;
    ULONG signals[ SIGNAL_SRC_COUNT ];
    ULONG total = 0L;
    WORD i;
    BOOL done = FALSE;
    UBYTE toggleFrame = sd->toggleFrame;

    signals[ SIGNAL_SRC_COPPER ] = 1L << sd->cop.signal;
    signals[ SIGNAL_SRC_SAFE ] = 1L << sd->safePort->mp_SigBit;
    signals[ SIGNAL_SRC_USER ] = 1L << sd->userPort->mp_SigBit;

    for( i = 0; i < SIGNAL_SRC_COUNT; i++ )
    {
        total |= signals[ i ];
    }

    while( !done )
    {
        ULONG result = Wait( total );

        if( result & signals[ SIGNAL_SRC_COPPER ] )
        {
            if( sd->safeToDraw )
            {
                struct windowMsg wm = { WINDOW_MSG_ANIMATE };
                WaitBlit();
                while( !ChangeScreenBuffer( sd->screen, sd->buffers[ toggleFrame ] ) )
                {
                    WaitTOF();
                }
                sd->toggleFrame = toggleFrame ^= 1;
                sd->safeToDraw = FALSE;
                if( backwd->handle )
                {
                    backwd->handle( backwd, &wm );
                }
            }
        }

        if( result & signals[ SIGNAL_SRC_SAFE ] )
        {
            if( !sd->safeToDraw )
            {
                while( !GetMsg( sd->safePort ) )
                {
                    WaitPort( sd->safePort );
                }
                sd->safeToDraw = TRUE;
            }
            windowUpdate( sd, backwd );
        }

        if( result & signals[ SIGNAL_SRC_USER ] )
        {
            struct IntuiMessage *msg;
            struct windowMsg wm = { WINDOW_MSG_USER };

            while( wm.imsg = msg = ( struct IntuiMessage * ) GetMsg( sd->userPort ) )
            {
                struct Window *w = msg->IDCMPWindow;
                struct windowData *wd = ( struct windowData * ) w->UserData;

                if( wd->handle )
                {
                    wd->handle( wd, &wm );
                }
                ReplyMsg( ( struct Message * ) msg );

                if( wd != backwd && wd->close )
                {
                    windowClose( wd );
                }
            }

            done = backwd->close;
        }
    }
    return( 0 );
}

struct BitMap *loadGfx( STRPTR name, struct Screen *s )
{
    struct IFFHandle *iff;

    if( iff = openIFF( name, IFFF_READ ) )
    {
        if( scanILBM( iff ) )
        {
            if( loadCMAP( iff, s ) )
            {
                struct BitMap *bm;
                if( bm = loadBitMap( iff ) )
                {
                    closeIFF( iff );
                    return( bm );
                }
            }
        }
        closeIFF( iff );
    }
    return( NULL );
}

LONG gameStart( struct WBStartup *wbs )
{
    if( libsOpen() )
    {
        struct screenData *sd;

        if( sd = screenOpen( DEPTH ) )
        {
            struct windowData *backwd;
            struct Rectangle rect = { 0, 0, 319, 255 };

            if( sd->backwd = backwd = windowOpen( sd, WINDOW_KIND_BACKDROP, &rect, backHandler ) )
            {
                if( sd->gfx = loadGfx( "Data1/Gfx/Graphics.iff", sd->screen ) )
                {
                    gameLoop( sd );
                    FreeBitMap( sd->gfx );
                }
                windowClose( backwd );
            }
            screenClose( sd );
        }
        libsClose();
    }
    return( 0 );
}

int main( void )
{
    return( gameStart( NULL ) );
}
