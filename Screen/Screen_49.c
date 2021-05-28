
/* Maintain screen */

/* $Log$ */

#include <intuition/intuition.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <graphics/gfxmacros.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>

#include <clib/exec_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>

#include "screen.h"

__far extern struct Custom custom;

extern void my_copper( void );

BOOL add_copper( struct screen *s )
{
    if( s->is = AllocMem( sizeof( *s->is ), MEMF_PUBLIC ) )
    {
        if( ( s->cop.signal = AllocSignal( -1 ) ) != -1 )
        {
            struct UCopList *ucl;

            s->cop.vp = &s->s->ViewPort;
            s->cop.task = FindTask( NULL );

            s->is->is_Code = my_copper;
            s->is->is_Data = ( APTR ) &s->cop;
            s->is->is_Node.ln_Pri = 0;

            if( ucl = AllocMem( sizeof( *ucl ), MEMF_PUBLIC | MEMF_CLEAR ) )
            {
                const UBYTE length = 3;

                CINIT( ucl, length );
                CWAIT( ucl, 0, 0 );
                CMOVE( ucl, custom.intreq, INTF_SETCLR | INTF_COPER );
                CEND( ucl );

                Forbid();
                s->s->ViewPort.UCopIns = ucl;
                Permit();

                RethinkDisplay();

                AddIntServer( INTB_COPER, s->is );

                return( TRUE );
            }
            FreeSignal( s->cop.signal );
        }
        FreeMem( s->is, sizeof( *s->is ) );
    }
    return( FALSE );
}

void rem_copper( struct screen *s )
{
    RemIntServer( INTB_COPER, s->is );
    FreeSignal( s->cop.signal );
    FreeMem( s->is, sizeof( *s->is ) );
}

BOOL open_screen( struct screen *s, UWORD width, UWORD height, UBYTE depth, struct TextAttr *ta, struct Rectangle *dclip, ULONG modeID, ULONG *colors, STRPTR title, ULONG idcmp )
{
    if( s->bm[ 0 ] = AllocBitMap( width, height, depth, BMF_DISPLAYABLE | BMF_INTERLEAVED | BMF_CLEAR, NULL ) )
    {
        if( s->bm[ 1 ] = AllocBitMap( width, height, depth, BMF_DISPLAYABLE | BMF_INTERLEAVED | BMF_CLEAR, NULL ) )
        {
            if( s->tf = OpenDiskFont( ta ) )
            {
                if( s->s = OpenScreenTags( NULL,
                    SA_Font,        ta,
                    SA_DClip,       dclip,
                    SA_DisplayID,   modeID,
                    SA_BitMap,      s->bm[ 0 ],
                    SA_Colors32,    colors,
                    SA_Title,       title,
                    SA_ShowTitle,   FALSE,
                    SA_Quiet,       TRUE,
                    SA_Exclusive,   TRUE,
                    SA_BackFill,    LAYERS_NOBACKFILL,
                    TAG_DONE ) )
                {
                    if( s->w = OpenWindowTags( NULL,
                        WA_CustomScreen,    s->s,
                        WA_Left,            0,
                        WA_Top,             0,
                        WA_Width,           s->s->Width,
                        WA_Height,          s->s->Height,
                        WA_Backdrop,        TRUE,
                        WA_Borderless,      TRUE,
                        WA_Activate,        TRUE,
                        WA_RMBTrap,         TRUE,
                        WA_SimpleRefresh,   TRUE,
                        WA_BackFill,        LAYERS_NOBACKFILL,
                        WA_ReportMouse,     TRUE,
                        WA_IDCMP,           idcmp,
                        TAG_DONE ) )
                    {
                        if( add_copper( s ) )
                        {
                            s->s->UserData = (APTR)s;
                            return( TRUE );
                        }
                        CloseWindow( s->w );
                    }
                    CloseScreen( s->s );
                }
                CloseFont( s->tf );
            }
            FreeBitMap( s->bm[ 1 ] );
        }
        FreeBitMap( s->bm[ 0 ] );
    }
    return( FALSE );
}

void close_screen( struct screen *s )
{
    rem_copper( s );
    CloseWindow( s->w );
    CloseScreen( s->s );
    CloseFont( s->tf );
    FreeBitMap( s->bm[ 1 ] );
    FreeBitMap( s->bm[ 0 ] );
}
