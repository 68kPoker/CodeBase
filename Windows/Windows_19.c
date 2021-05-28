
/* $Id$ */

#include "Windows.h"
#include "Screen.h"

#include <intuition/intuition.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/layers_protos.h>

VOID windowUpdate( struct screenData *sd, struct windowData *wd )
{
    struct windowMsg wm = { WINDOW_MSG_DRAW };
    struct Region *prevClip;
    UBYTE toggleFrame = sd->toggleFrame;

    if( wd->handle && ( wd->flags & 0x3 ) )
    {
        OrRegionRegion( wd->update[ toggleFrame ], wd->update[ toggleFrame ^ 1 ] );

        prevClip = InstallClipRegion( wd->window->WLayer, wd->update[ toggleFrame ^ 1 ] );

        wd->window->RPort->BitMap = sd->buffers[ toggleFrame ]->sb_BitMap;
        wd->handle( wd, &wm );

        InstallClipRegion( wd->window->WLayer, prevClip );

        ClearRegion( wd->update[ toggleFrame ^ 1 ] );

        wd->flags &= ~( 1 << toggleFrame );
    }
}

struct windowData *windowOpen( struct screenData *sd, WORD kind, struct Rectangle *rect, handleWindow handle )
{
    WORD left   = rect->MinX,
         top    = rect->MinY,
         width  = rect->MaxX - left + 1,
         height = rect->MaxY - top + 1;

    struct windowData *wd;
    ULONG idcmp[] =
    {
        IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_REFRESHWINDOW,
        IDCMP_RAWKEY | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE
    };

    if( wd = AllocMem( sizeof( *wd ), MEMF_PUBLIC | MEMF_CLEAR ) )
    {
        wd->bounds = *rect;
        wd->handle = handle;

        if( wd->window = OpenWindowTags( NULL,
            WA_CustomScreen,    sd->screen,
            WA_Left,            left,
            WA_Top,             top,
            WA_Width,           width,
            WA_Height,          height,
            WA_Backdrop,        kind == WINDOW_KIND_BACKDROP,
            WA_Borderless,      TRUE,
            WA_Activate,        TRUE,
            WA_RMBTrap,         TRUE,
            WA_SimpleRefresh,   TRUE,
            WA_BackFill,        LAYERS_NOBACKFILL,
            WA_IDCMP,           0,
            WA_ReportMouse,     TRUE,
            WA_AutoAdjust,      FALSE,
            TAG_DONE ) )
        {
            if( wd->update[ 0 ] = NewRegion() )
            {
                if( wd->update[ 1 ] = NewRegion() )
                {
                    struct windowMsg wm = { WINDOW_MSG_INIT };

                    wd->window->UserData = ( APTR ) wd;
                    wd->window->UserPort = sd->userPort;
                    ModifyIDCMP( wd->window, idcmp[ kind ] );

                    wd->handle( wd, &wm );

                    windowUpdate( sd, wd );

                    return( wd );
                }
                DisposeRegion( wd->update[ 0 ] );
            }
            CloseWindow( wd->window );
        }
        FreeMem( wd, sizeof( *wd ) );
    }
    return( NULL );
}

VOID stripIntuiMessages( struct MsgPort *mp, struct Window *w )
{
    struct IntuiMessage *msg;
    struct Node *succ;

    msg = ( struct IntuiMessage * ) mp->mp_MsgList.lh_Head;

    while( succ = msg->ExecMessage.mn_Node.ln_Succ )
    {
        if( msg->IDCMPWindow == w )
        {
            Remove( ( struct Node * ) msg );
            ReplyMsg( ( struct Message * ) msg );
        }
        msg = ( struct IntuiMessage * ) succ;
    }
}

VOID windowClose( struct windowData *wd )
{
    DisposeRegion( wd->update[ 1 ] );
    DisposeRegion( wd->update[ 0 ] );

    Forbid();
    stripIntuiMessages( wd->window->UserPort, wd->window );
    wd->window->UserPort = NULL;
    ModifyIDCMP( wd->window, 0 );
    Permit();

    CloseWindow( wd->window );
    FreeMem( wd, sizeof( *wd ) );
}
