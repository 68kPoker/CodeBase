
/* $Id$ */

#include <intuition/screens.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/iffparse.h>
#include <intuition/imageclass.h>

#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>

#include "Screen.h"
#include "Screen_protos.h"

#include "IFF.h"
#include "IFF_protos.h"

#include "Images_protos.h"

/* Open screen with given palette, bitmap, mode and dimensions */

BOOL openScreen( struct screenInfo *si, ULONG *pal, struct BitMap *bm, ULONG mode, struct Rectangle *dclip )
{
    si->bitmaps[ 0 ] = bm;
    UWORD pens[] = { ~0 };

    WaitBlit();
    if( si->screen = OpenScreenTags( NULL,
        SA_DisplayID,   mode,
        SA_DClip,       dclip,
        SA_BitMap,      bm,
        pal ? SA_Colors32 : TAG_IGNORE,    pal,
        SA_Exclusive,   TRUE,
        SA_Quiet,       TRUE,
        SA_ShowTitle,   FALSE,
        SA_BackFill,    LAYERS_NOBACKFILL,
        SA_Pens,        pens,
        TAG_DONE ) )
    {
        /* Alloc bitmap */

        if( si->bitmaps[ 1 ] = AllocBitMap(
            GetBitMapAttr( bm, BMA_WIDTH ),
            GetBitMapAttr( bm, BMA_HEIGHT ),
            GetBitMapAttr( bm, BMA_DEPTH ),
            BMF_DISPLAYABLE | BMF_INTERLEAVED,
            NULL ) )
        {
            if( si->scrBuffers[ 0 ] = AllocScreenBuffer( si->screen, si->bitmaps[ 0 ], 0 ) )
            {
                if( si->scrBuffers[ 1 ] = AllocScreenBuffer( si->screen, si->bitmaps[ 1 ], SB_COPY_BITMAP ) )
                {
                    if( si->safePort = CreateMsgPort() )
                    {
                        si->safeToDraw = TRUE;
                        si->curBuffer = 1;
                        si->scrBuffers[ 0 ]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = si->safePort;
                        si->scrBuffers[ 1 ]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = si->safePort;
                        si->screen->RastPort.BitMap = si->scrBuffers[ si->curBuffer ]->sb_BitMap;

                        si->screen->UserData = ( APTR ) si;

                        if( si->window = OpenWindowTags( NULL,
                            WA_CustomScreen,    si->screen,
                            WA_Left,            0,
                            WA_Top,             0,
                            WA_Width,           si->screen->Width,
                            WA_Height,          si->screen->Height,
                            WA_Backdrop,        TRUE,
                            WA_Borderless,      TRUE,
                            WA_Activate,        TRUE,
                            WA_RMBTrap,         TRUE,
                            WA_SimpleRefresh,   TRUE,
                            WA_BackFill,        LAYERS_NOBACKFILL,
                            WA_IDCMP,           IDCMP_GADGETDOWN|IDCMP_GADGETUP|IDCMP_RAWKEY,
                            TAG_DONE ) )
                        {
                            return( TRUE );
                        }
                        DeleteMsgPort( si->safePort );
                    }
                    FreeScreenBuffer( si->screen, si->scrBuffers[ 1 ] );
                }
                FreeScreenBuffer( si->screen, si->scrBuffers[ 0 ] );
            }
            FreeBitMap( si->bitmaps[ 1 ] );
        }
        CloseScreen( si->screen );
    }
    return( FALSE );
}

void closeScreen( struct screenInfo *si )
{
    CloseWindow( si->window );
    if( !si->safeToDraw )
    {
        while( !GetMsg( si->safePort ) )
        {
            WaitPort( si->safePort );
        }
    }
    DeleteMsgPort( si->safePort );
    FreeScreenBuffer( si->screen, si->scrBuffers[ 1 ] );
    FreeScreenBuffer( si->screen, si->scrBuffers[ 0 ] );
    CloseScreen( si->screen );
    FreeBitMap( si->bitmaps[ 1 ] );
}

int main( int argc, char **argv )
{
    /* Small test */
    struct Rectangle dclip = { 0, 0, 319, 255 };
    ULONG mode = LORES_KEY;
    struct BitMap *bm;
    struct screenInfo si;
    struct IFFHandle *iff;
    struct ILBMInfo ii;
    BPTR f;
    STRPTR gfxName;
    struct Image image, selected;
    struct Rectangle myClip = { 0, 0, 15, 15 };

    if( argc < 2 )
        return( 0 );

    gfxName = argv[ 1 ];

    if( bm = AllocBitMap( 320, 256, 5, BMF_DISPLAYABLE | BMF_INTERLEAVED | BMF_CLEAR, NULL ) )
    {
        if( iff = AllocIFF() )
        {
            if( f = Open( gfxName, MODE_OLDFILE ) )
            {
                if( openIFF( iff, f, TRUE, IFFF_READ ) )
                {
                    if( readILBM( &ii, iff ) )
                    {
                        if( cutImage( &image, ii.brush, &myClip ) )
                        {
                            image.LeftEdge = image.TopEdge = 1;
                            myClip.MinY += 16;
                            myClip.MaxY += 16;
                            cutImage( &selected, ii.brush, &myClip );
                            if( openScreen( &si, ii.palette, bm, mode, &dclip ) )
                            {
                                Object *frame, *gad;

                                frame = NewObject( NULL, "frameiclass",
                                    IA_Width, 18,
                                    IA_Height, 18,
                                    IA_EdgesOnly, TRUE,
                                    TAG_DONE);

                                if( gad = NewObject( NULL, "frbuttonclass",
                                    GA_Left, 0,
                                    GA_Top,  0,
                                    GA_Width, 16,
                                    GA_Height, 16,
                                    GA_Image, frame,
                                    GA_LabelImage, &image,
                                    GA_Immediate, TRUE,
                                    GA_RelVerify, TRUE,
                                    TAG_DONE))
                                {
                                    BOOL done = FALSE;

                                    WaitBlit();
                                    while( !ChangeScreenBuffer( si.screen, si.scrBuffers[ 1 ] ) )
                                    {
                                        WaitTOF();
                                    }
                                    AddGadget( si.window, ( struct Gadget * ) gad, -1 );
                                    while( !done )
                                    {
                                        struct IntuiMessage *msg;

                                        WaitPort( si.window->UserPort );

                                        while( msg = ( struct IntuiMessage * ) GetMsg( si.window->UserPort ) )
                                        {
                                            if( msg->Class == IDCMP_GADGETDOWN)
                                            {
                                            }
                                            else if( msg->Class == IDCMP_GADGETUP )
                                            {
                                                struct Gadget *gad = ( struct Gadget * ) msg->IAddress;
                                            }
                                            else
                                                done = TRUE;


                                            ReplyMsg( ( struct Message * ) msg );
                                        }
                                    }

                                    RemoveGadget( si.window, ( struct Gadget * ) gad );
                                    DisposeObject( gad );
                                }
                                closeScreen( &si );
                            }
                            FreeVec( selected.ImageData );
                            FreeVec( image.ImageData );
                        }
                        FreeBitMap( ii.brush );
                        FreeVec( ii.palette );
                    }
                    CloseIFF( iff );
                }
                Close( f );
            }
            FreeIFF( iff );
        }
        FreeBitMap( bm );
    }
    return( 0 );
}
