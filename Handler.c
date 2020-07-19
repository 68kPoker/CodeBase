
#include <stdio.h>

#include <intuition/intuition.h>

#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/gadtools_protos.h>

#include "IFF.h"

#define KEY_ESC    0x45

#define SCR_WIDTH  320
#define SCR_HEIGHT 256
#define SCR_DEPTH  5
#define SCR_MODEID LORES_KEY

struct WindowData {
    struct Window *w;
    BOOL esc;
};

/*
 * Open screen and backdrop window
 */

struct Window *OpenMyWindow( struct WindowData *wd )
{
    struct Screen *s;

    if( s = OpenScreenTags( NULL,
        SA_Left,        0,
        SA_Top,         0,
        SA_Width,       SCR_WIDTH,
        SA_Height,      SCR_HEIGHT,
        SA_Depth,       SCR_DEPTH,
        SA_DisplayID,   SCR_MODEID,
        SA_Quiet,       TRUE,
        SA_Exclusive,   TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        TAG_DONE ) ) {

        struct Window *w;

        if( w = OpenWindowTags( NULL,
            WA_CustomScreen,    s,
            WA_Left,            0,
            WA_Top,             0,
            WA_Width,           s->Width,
            WA_Height,          s->Height,
            WA_Backdrop,        TRUE,
            WA_Borderless,      TRUE,
            WA_Activate,        TRUE,
            WA_RMBTrap,         TRUE,
            WA_BackFill,        LAYERS_NOBACKFILL,
            WA_SimpleRefresh,   TRUE,
            WA_NoCareRefresh,   TRUE,
            WA_IDCMP,           IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
            WA_ReportMouse,     TRUE,
            TAG_DONE ) ) {

            w->UserData = ( APTR ) wd;
            wd->w = w;

            return( w );
        }
        CloseScreen( s );
    }
    return( NULL );
}

void CloseMyWindow( struct Window *w )
{
    struct Screen *s = w->WScreen;

    CloseWindow( w );
    CloseScreen( s );
}

/*
 * Handle all messages
 */

HandleAll( struct Window *w )
{
    struct WindowData *wd = ( struct WindowData * ) w->UserData;

    while( !wd->esc ) {
        ULONG total = 1L << wd->w->UserPort->mp_SigBit;
        ULONG result = Wait( total );

        if( result & ( 1L << wd->w->UserPort->mp_SigBit ) )
            HandleUserPort( wd->w );
    }
}

/*
 * Handle UserPort of specified Intuition Window
 */

HandleUserPort( struct Window *w )
{
    struct MsgPort      *up = w->UserPort;
    struct IntuiMessage *msg;

    /* Obtain, handle and reply intuition messages */

    while( msg = GT_GetIMsg( up ) ) {
        HandleIntuiMessage( msg );
        GT_ReplyIMsg( msg );
    }
}

/*
 * Handle specified IntuiMessage
 */

HandleIntuiMessage( struct IntuiMessage *msg )
{
    struct Window *w = msg->IDCMPWindow;

    switch( msg->Class ) {
        case IDCMP_RAWKEY:
            HandleRawKey( msg );
            break;

        case IDCMP_MOUSEBUTTONS:
        case IDCMP_MOUSEMOVE:
            HandleMouse( msg );
            break;
    }
}

/*
 * Handle raw key
 */

HandleRawKey( struct IntuiMessage *msg )
{
    struct Window *w = msg->IDCMPWindow;
    struct WindowData *wd = ( struct WindowData * ) w->UserData;

    switch( msg->Code ) {
        case KEY_ESC:
            wd->esc = TRUE;
            break;
    }
}

/*
 * Handle mouse event
 */

HandleMouse( struct IntuiMessage *msg )
{
    struct Window *w = msg->IDCMPWindow;
    struct WindowData *wd = ( struct WindowData * ) w->UserData;

    switch( msg->Code ) {
        case IECODE_LBUTTON:
            printf("LButton\n");
            break;

        case IECODE_LBUTTON|IECODE_UP_PREFIX:
            printf("LButton up\n");
            break;

        case IECODE_RBUTTON:
            printf("RButton\n");
            break;

        case IECODE_RBUTTON|IECODE_UP_PREFIX:
            printf("RButton up\n");
            break;

        case IECODE_NOBUTTON:
            printf("NoButton\n");
            break;
    }
}

int main()
{
    struct Window *w;
    struct WindowData wd = { 0 };
    struct BitMap *gfx;

    if( w = OpenMyWindow( &wd ) ) {
        if( gfx = LoadPicture( "Data/Scenario1.iff", &w->WScreen->ViewPort.ColorMap ) ) {
            MakeScreen( w->WScreen );
            RethinkDisplay();

            BltBitMapRastPort( gfx, 0, 0, w->RPort, 0, 0, 320, 256, 0xc0 );

            HandleAll( w );
            UnloadPicture( gfx, NULL );
        }
        CloseMyWindow( w );
    }
    return( 0 );
}
