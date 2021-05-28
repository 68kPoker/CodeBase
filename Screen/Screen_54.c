
#include <stdlib.h>

#include <intuition/screens.h>
#include <clib/intuition_protos.h>

#include "CScreen.h"
#include "CIFF.h"

BOOL openScreen( struct CScreen *scr, struct CILBM *ilbm )
{
    struct Screen *s;
    struct Rectangle dclip = { 0, 0, 319, 255 };
    ULONG triplet[ 3 ];
    WORD i;

    if( s = OpenScreenTags( NULL,
        SA_DClip, &dclip,
        SA_Depth, ilbm->bmhd.bmhd.bmh_Depth,
        SA_DisplayID, LORES_KEY,
        SA_Quiet, TRUE,
        SA_Exclusive, TRUE,
        SA_ShowTitle, FALSE,
        TAG_DONE ) ) {

        for( i = 0; i < ilbm->cmap.cm->Count; i++ ) {
            GetRGB32( ilbm->cmap.cm, i, 1, triplet );
            SetRGB32CM( s->ViewPort.ColorMap, i, triplet[ 0 ], triplet[ 1 ], triplet[ 2 ] );
        }
        MakeScreen( s );
        RethinkDisplay();

        scr->s = s;
        scr->ilbm = ilbm;

        return( TRUE );
    }
    return( FALSE );
}

void closeScreen( struct CScreen *s )
{
    CloseScreen( s->s );
}

int main( void )
{
    struct CILBM ilbm = { 0 };
    struct CScreen screen;

    if( loadILBM( "Data/Game.iff", &ilbm ) == 0 ) {
        if( openScreen( &screen, &ilbm ) ) {
            BltBitMapRastPort( ilbm.bm, 0, 0, &screen.s->RastPort, 0, 0, 64, 16, 0xc0 );
            BltBitMapRastPort( ilbm.bm, 0, 0, &screen.s->RastPort, 0, 16, 64, 16, 0xc0 );
            BltBitMapRastPort( ilbm.bm, 0, 0, &screen.s->RastPort, 0, 32, 64, 16, 0xc0 );
            BltBitMapRastPort( ilbm.bm, 0, 0, &screen.s->RastPort, 0, 48, 64, 16, 0xc0 );
            BltBitMapRastPort( ilbm.bm, 0, 0, &screen.s->RastPort, 0, 64, 64, 16, 0xc0 );
            BltBitMapRastPort( ilbm.bm, 0, 128, &screen.s->RastPort, 0, 80, 64, 32, 0xc0 );

            WORD x, y, z;

            srand( 100 );

            for( x = 0; x < 15; x++)
            {
                for( z = 0; z < 16; z += 2 )
                {
                    for( y = 0; y < 16; y++ )
                    {
                        BltBitMapRastPort( ilbm.bm, 16, 128, &screen.s->RastPort, 64 + ( x << 4 ) + z + 1, y << 4, 16, 16, 0xc0 );

                        if( x > 0 && y > 0 && y < 15 ) {
                            if( x == 8 )
                                BltBitMapRastPort( ilbm.bm, 32 + z, 144, &screen.s->RastPort, 64 + ( x << 4 ) + z, y << 4, 2, 16, 0xc0 );
                            else
                                BltBitMapRastPort( ilbm.bm, 0 + z, 128, &screen.s->RastPort, 64 + ( x << 4 ) + z, y << 4, 2, 16, 0xc0 );
                        }
                        else
                            BltBitMapRastPort( ilbm.bm, 16 + z, 128, &screen.s->RastPort, 64 + ( x << 4 ) + z, y << 4, 2, 16, 0xc0 );
                    }
                    WaitTOF();
                }
            }

            Delay( 400 );
            closeScreen( &screen );
        }
        unloadILBM( &ilbm );
    }
    return( 0 );
}
