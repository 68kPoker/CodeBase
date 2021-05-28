
#include <dos/dos.h>
#include <intuition/screens.h>

#include <clib/dos_protos.h>
#include <clib/graphics_protos.h>

#include "Screen.h"
#include "Data.h"
#include "Blitter.h"

VOID test( struct Screen *s, struct BitMap *gfx )
{
    struct BitMap *dest = s->RastPort.BitMap;
    PLANEPTR mask;

    if( mask = createMask( gfx ) )
    {
        /* Blit icon test */
        blitIcon( gfx, dest, 0, 0, 0, 0, 160, 128 );

        Delay( 200 );

        /* Blit Bob test */
        blitBob( gfx, dest, 80, 0, 84, 4, 16, 16, mask );

        Delay( 300 );
        freeMask( mask, gfx );
    }
}

int main()
{
    struct Screen *s;

    if( s = openScreen() )
    {
        struct BitMap *gfx;

        if( gfx = loadBitMap( s, "Dane/Grafika.iff" ) )
        {
            test( s, gfx );
            FreeBitMap( gfx );
        }
        closeScreen( s );
    }
    return( 0 );
}
