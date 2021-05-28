
/* General screen related functions */

#include <intuition/screens.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <graphics/gfxmacros.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/intuition_protos.h>

#include "Screen.h"

extern ULONG myCopperCode(); /* Copper server */

extern __far struct Custom custom;

UBYTE version[] = "$VER: Magazyn 1.0";

BOOL initCopper( struct screenUser *su )
{
    struct UCopList *ucl;

    if( !( su->task = FindTask( NULL ) ) )
        printf( "Couldn't find task!\n" );
    else
    {
        if( ( su->copperSignal = AllocSignal( -1 ) ) == -1 )
            printf( "Couldn't alloc signal!\n" );
        else
        {
            if( !( ucl = AllocMem( sizeof( *ucl ), MEMF_PUBLIC | MEMF_CLEAR ) ) )
                printf( "Couldn't alloc mem!\n" );
            else
            {
                CINIT( ucl, COPPER_COMMANDS );
                CWAIT( ucl, 0, 0 );
                CMOVE( ucl, custom.intreq, INTF_SETCLR | INTF_COPER );
                CEND( ucl );

                su->copper.is_Node.ln_Name = version;
                su->copper.is_Node.ln_Pri  = COPPER_PRIORITY;
                su->copper.is_Code = (void(*)())myCopperCode;
                su->copper.is_Data = ( APTR ) &su->task;

                AddIntServer( INTB_COPER, &su->copper );

                Forbid();

                su->screen->ViewPort.UCopIns = ucl;

                Permit();

                return( TRUE );
            }
            FreeSignal( su->copperSignal );
        }
    }
    return( FALSE );
}

VOID freeCopper( struct screenUser *su )
{
    RemIntServer( INTB_COPER, &su->copper );
    FreeSignal( su->copperSignal );
}

BOOL initScreen( struct screenUser *su, struct screenParam *sp )
{
    if( !( su->font = OpenDiskFont( &sp->textAttr ) ) )
        printf( "Unable to open %s size %d!\n", sp->textAttr.ta_Name, sp->textAttr.ta_YSize );
    else
    {
        su->bitmaps[ 0 ] = sp->bitmaps[ 0 ];
        su->bitmaps[ 1 ] = sp->bitmaps[ 1 ];

        if( !( su->screen = OpenScreenTags( NULL,
            SA_DClip,       &sp->displayClip,
            SA_DisplayID,   sp->modeID,
            SA_BitMap,      sp->bitmaps[ 0 ],
            SA_Colors32,    sp->paletteRGB,
            SA_Font,        &sp->textAttr,
            SA_Title,       sp->title,
            SA_Quiet,       TRUE,
            SA_Exclusive,   TRUE,
            SA_ShowTitle,   FALSE,
            SA_Draggable,   FALSE,
            SA_BackFill,    LAYERS_NOBACKFILL,
            TAG_DONE ) ) )
            printf( "Unable to open screen!\n" );
        else
        {
            su->screen->UserData = ( APTR ) su;
            if( !( su->buffers[ 0 ] = AllocScreenBuffer( su->screen, su->bitmaps[ 0 ], 0 ) ) )
                printf( "Unable to alloc screen buffer!\n" );
            else
            {
                if( !( su->buffers[ 1 ] = AllocScreenBuffer( su->screen, su->bitmaps[ 1 ], 0 ) ) )
                    printf( "Unable to alloc screen buffer!\n" );
                else
                {
                    if( !( su->safePort = CreateMsgPort() ) )
                        printf( "Unable to create message port!\n" );
                    else
                    {
                        su->buffers[ 0 ]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = su->safePort;
                        su->buffers[ 1 ]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = su->safePort;

                        su->safeToWrite = TRUE;
                        su->frame = 1;
                        if( initCopper( su ) )
                        {
                            return( TRUE );
                        }
                        DeleteMsgPort( su->safePort );
                    }
                    FreeScreenBuffer( su->screen, su->buffers[ 1 ] );
                }
                FreeScreenBuffer( su->screen, su->buffers[ 0 ] );
            }
            CloseScreen( su->screen );
        }
        CloseFont( su->font );
    }
    return( FALSE );
}

VOID safeToWrite( struct screenUser *su )
{
    if( !su->safeToWrite )
    {
        while( !GetMsg( su->safePort ) )
        {
            WaitPort( su->safePort );
        }
        su->safeToWrite = TRUE;
    }
}

/* Call upon Copper interrupt */
VOID changeScreen( struct screenUser *su )
{
    UWORD frame = su->frame;

    WaitBlit();
    while( !ChangeScreenBuffer( su->screen, su->buffers[ frame ] ) )
    {
        WaitTOF();
    }

    su->frame = frame ^ 1;
    su->safeToWrite = FALSE;
}

VOID freeScreen( struct screenUser *su )
{
    freeCopper( su );

    safeToWrite( su );
    DeleteMsgPort( su->safePort );
    FreeScreenBuffer( su->screen, su->buffers[ 1 ] );
    FreeScreenBuffer( su->screen, su->buffers[ 0 ] );
    CloseScreen( su->screen );
    CloseFont( su->font );
    FreeBitMap( su->bitmaps[ 1 ] );
    FreeBitMap( su->bitmaps[ 0 ] );
}
