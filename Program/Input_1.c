
#include <stdio.h>

#include <dos/dos.h>
#include <exec/types.h>
#include <exec/memory.h>

#include <intuition/intuition.h>
#include <clib/intuition_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "CInput.h"
#include "IFF.h"
#include "Chunky.h"
#include "CGUI.h"

BOOL done = FALSE;

STRPTR menu[] = { "Gra", "Edytor", "Kafelek", "Opcje" };
struct BitMap *gfx;
CScreen screen;

void initInput( CInput *in, ULONG signalMask, CInputHandler handler, APTR user )
{
    in->signalMask = signalMask;
    in->handler    = handler;
    in->user       = user;
}

void initIDCMP( CInput *in, CIDCMP *idcmp, struct MsgPort *userPort, CInputHandler handler )
{
    initInput( in, 1L << userPort->mp_SigBit, handler, idcmp );
    idcmp->userPort = userPort;
}

void initCopper( CInput *in, CCopper *cop, CInputHandler handler )
{
    initInput( in, 1L << cop->signal, handler, cop );
}

void initSafe( CInput *in, CSafeToDraw *safe, CInputHandler handler )
{
    initInput( in, 1L << safe->mp->mp_SigBit, handler, safe );
}

ULONG sumInput( CInputs inputs )
{
    WORD i;
    ULONG signalMask = 0L;
    CInput *in = inputs;

    for( i = 0; i < INPUTS; i++, in++ )
        signalMask |= in->signalMask;

    return( signalMask );
}

void handleInput( CInputs inputs )
{
    ULONG signalMask = sumInput( inputs );
    ULONG resultMask = Wait( signalMask );
    WORD i;
    CInput *in = inputs;

    for( i = 0; i < INPUTS; i++, in++ )
        if( resultMask & in->signalMask )
            in->handler( in->user );
}

void game( CInputs inputs )
{
    while( !done )
        handleInput( inputs );
}

void handleMessage( struct IntuiMessage *msg )
{
    static WORD active = -1;
    static BOOL status = FALSE;
    struct RastPort *rp = msg->IDCMPWindow->RPort;

    switch( msg->Class ) {
        case IDCMP_RAWKEY:
            switch( msg->Code ) {
                case ESC_KEY:
                    done = TRUE;
                    break;
            }
            break;
        case IDCMP_CLOSEWINDOW:
            done = TRUE;
            break;

        case IDCMP_MOUSEBUTTONS:
            if( msg->Code == IECODE_LBUTTON ) {
                if( msg->MouseY < 16 ) {
                    active = msg->MouseX / 80;
                    BltBitMapRastPort( gfx, 80, 0, rp, (msg->MouseX / 80) * 80, 0, 80, 16, 0xc0 );
                    Move( rp, ( active * 80 ) + 16, 4 + screen.font->tf_Baseline );
                    Text( rp, menu[ active ], strlen( menu[ active ] ) );
                    status = 1;
                }
            }
            else if( msg->Code == ( IECODE_LBUTTON | IECODE_UP_PREFIX ) ) {
                if( ( msg->MouseX / 80 ) == active ) {
                    /* Do */
                    Move( rp, ( active * 80 ) + 16, 4 + screen.font->tf_Baseline );
                    BltBitMapRastPort( gfx, 0, 0, rp, active * 80, 0, 80, 16, 0xc0 );
                    Text( rp, menu[ active ], strlen( menu[ active ] ) );
                    status = 0;
                }
                active = -1;
            }
            break;

        case IDCMP_MOUSEMOVE:
            if( active != -1 ) {
                if( ( status && ( ( msg->MouseX / 80 ) != active || msg->MouseY >= 16 ) )
                ||  ((  !status ) &&  (( msg->MouseX / 80 ) == active && msg->MouseY < 16 ) ) ) {

                    Move( rp, ( active * 80 ) + 16, 4 + screen.font->tf_Baseline );
                    BltBitMapRastPort( gfx, status ? 0 : 80, 0, rp, active * 80, 0, 80, 16, 0xc0 );
                    Text( rp, menu[ active ], strlen( menu[ active ] ) );
                    status ^= 1;
                }
            }
            break;
    }
}

void simpleHandler( CIDCMP *idcmp )
{
    struct MsgPort *userPort = idcmp->userPort;
    struct IntuiMessage *msg;

    while( msg = ( struct IntuiMessage * ) GetMsg( userPort ) ) {
        handleMessage( msg );
        ReplyMsg( ( struct Message * ) msg );
    }
}

void drawHandler( CSafeToDraw *safe )
{
/*
    UBYTE text[ 5 ];
    static WORD counter = 0;
*/

    return;

    if( !safe->safe ) {
        while( !GetMsg( safe->mp ) )
            WaitPort( safe->mp );
        safe->safe = TRUE;
    }

/*
    struct RastPort *rp = safe->s->backw->RPort;
    WORD frame = safe->s->frame;

    rp->BitMap = safe->s->sb[ frame ]->sb_BitMap;

    sprintf( text, "%4d", counter++ );

    SetAPen( rp, 1 );
    Move( rp, 0, rp->Font->tf_Baseline );
    Text( rp, text, 4 );
*/
}

void animHandler( CCopper *cop )
{
    WORD frame = cop->s->frame;

    if( cop->s->safe.safe ) {
        WaitBlit();

        if( ChangeScreenBuffer( cop->s->s, cop->s->sb[ frame ] ) ) {
            cop->s->frame = frame ^= 1;
            cop->s->safe.safe = FALSE;
        }
    }
}

void simpleJoystick( CController *con )
{
    printf( "$%2x %d %d\n", con->ie.ie_Code, con->ie.ie_X, con->ie.ie_Y );

    readEvent( con );
}

void demo( void )
{
    CInputs inputs = { 0 };
    /* CController con; */

    struct ColorMap *cm;
    ULONG *pal;
    struct RastPort *rp;

    if( openFont( &screen ) ) {
        if( openScreen( inputs, &screen, NULL, drawHandler, animHandler ) ) {
            if( openBackWindow( inputs, &screen, simpleHandler ) ) {

                if( gfx = loadBitMap( "Data/Graphics.iff", &cm ) ) {
                    if( pal = AllocVec( ( ( cm->Count * 3 ) + 2 ) * sizeof( ULONG ), MEMF_PUBLIC ) ) {
                        pal[ 0 ] = cm->Count << 16;
                        pal[ ( cm->Count * 3 ) + 1 ] = 0L;

                        GetRGB32( cm, 0, cm->Count, pal + 1 );

                        LoadRGB32( &screen.s->ViewPort, pal );

                        WORD i;
                        rp = screen.backw->RPort;
                        rp->BitMap = screen.sb[ 1 ]->sb_BitMap;

                        for( i = 0; i < 4; i++ ) {
                            BltBitMapRastPort( gfx, 0, 0, rp, i * 80, 0, 80, 16, 0xc0 );
                            Move( rp, ( i * 80 ) + 16, 4 + screen.font->tf_Baseline );
                            SetABPenDrMd( rp, 4, 0, JAM1 );
                            Text( rp, menu[ i ], strlen( menu[ i ] ) );
                        }

                        game( inputs );

                        FreeVec( pal );
                    }
                    FreeBitMap( gfx );
                    FreeColorMap( cm );
                }
                closeBackWindow( &screen );
            }
            closeScreen( &screen );
        }
        closeFont( &screen );
    }
}

int main( void )
{
    demo();
    return( RETURN_OK );
}
