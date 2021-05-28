
#include <graphics/modeid.h>

#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>

#include "IFF.h"
#include "Screen.h"

BOOL openScreen( struct screenUser *su, struct Graphics *gfx )
{
    struct TextAttr ta =
    {
        "ld.font", 8, FS_NORMAL, FPF_DISKFONT | FPF_DESIGNED
    };
    struct screenParam sp;

    sp.displayClip.MinX = 0;
    sp.displayClip.MinY = 0;
    sp.displayClip.MaxX = 319;
    sp.displayClip.MaxY = 255;
    sp.modeID = LORES_KEY;
    sp.paletteRGB = gfx->colorsRGB32;
    sp.textAttr = ta;
    sp.title = "Magazyn";

    if( sp.bitmaps[ 0 ] = AllocBitMap( 320, 256, 5, BMF_DISPLAYABLE, NULL ) )
    {
        BltBitMap( gfx->bitmap, 0, 0, sp.bitmaps[ 0 ], 0, 0, 320, 256, 0xc0, 0xff, NULL );
        if( sp.bitmaps[ 1 ] = AllocBitMap( 320, 256, 5, BMF_DISPLAYABLE | BMF_CLEAR, NULL ) )
        {
            if( initScreen( su, &sp ) )
            {
                return( TRUE );
            }
            FreeBitMap( sp.bitmaps[ 1 ] );
        }
        FreeBitMap( sp.bitmaps[ 0 ] );
    }
    return( FALSE );
}

int main( void )
{
    struct Graphics *gfx;
    struct screenUser su;

    if( gfx = loadGraphics( "Data/Common.iff" ) )
    {
        if( openScreen( &su, gfx ) )
        {
            Delay( 500 );

            freeScreen( &su );
        }
        freeGraphics( gfx );
    }
    return( 0 );
}
