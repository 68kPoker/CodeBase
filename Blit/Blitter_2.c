
/* Blitter functions */

#include <exec/types.h>
#include <hardware/custom.h>
#include <hardware/blit.h>
#include <graphics/gfx.h>

#include <clib/graphics_protos.h>

#include "Blitter.h"

__far extern struct Custom custom;

/* blitModulo() - Calculates modulo of the operation */
/* Gets bitmap width (in bytes) and operation width (in words) */
/* Returns modulo */

WORD blitModulo( WORD rasWidth, WORD blitWidth )
{
    return( rasWidth - ( blitWidth << 1 ) );
}

/* blitShift() - Calculates channel shift value */
/* Gets destination X position in pixels */

UBYTE blitShift( WORD positionX )
{
    return( positionX & 0xF );
}

/* blitOffset() - Calculates channel offset value */
/* Gets X and Y position and BytesPerRow */

LONG blitOffset( WORD positionX, WORD positionY, WORD bpr )
{
    return( ( positionY * bpr ) + ( ( positionX >> 4 ) << 1 ) );
}

/* blitWidth() - Calculates word width of the operation */
/* Gets object width in pixels and shift value */
/* Returns width in words */

WORD blitWidth( WORD rasWidth, UBYTE shift )
{
    return( ( rasWidth + 15 + shift ) >> 4 );
}

/* wordWidth() - Calculates word width */

WORD blitWordWidth( WORD rasWidth )
{
    return( ( rasWidth + 15 ) >> 4 );
}

/* blitMask() - Calculates A channel last word mask */
/* Gets object width in pixels and shift value */
/* Returns mask */

UWORD blitMask( WORD rasWidth, UBYTE shift )
{
    if( blitWidth( rasWidth, shift ) > blitWordWidth( rasWidth ) )
        /* Mask out last word */
        return( 0 );

    return( 0xFFFF << ( 15 - ( ( rasWidth - 1 ) & 0xF ) ) );
}

/* blitSize() - Calculates blit size register */
/* Gets width in words and height in pixels */

UWORD blitSize( WORD wordWidth, WORD height )
{
    return( ( height << 6 ) | wordWidth );
}

/* blitIcon() - Draws icon */
/* Gets icons BitMap and destination BitMap, size of icon and destination
 * icon position. */

VOID blitIcon( struct BitMap *srcBitMap, struct BitMap *destBitMap, WORD srcX, WORD srcY, WORD destX, WORD destY, WORD width, WORD height )
{
    WORD plane, depth = destBitMap->Depth;
    struct Custom *cust = &custom;

    PLANEPTR *srcPlanes = srcBitMap->Planes, *destPlanes = destBitMap->Planes;

    WORD wordWidth = blitWordWidth( width );

    WORD srcBPR = srcBitMap->BytesPerRow, destBPR = destBitMap->BytesPerRow;
    WORD srcModulo = blitModulo( srcBPR, wordWidth );
    WORD destModulo = blitModulo( destBPR, wordWidth );

    LONG srcOffset = blitOffset( srcX, srcY, srcBPR );
    LONG destOffset = blitOffset( destX, destY, destBPR );

    UWORD size = blitSize( wordWidth, height );

    OwnBlitter();
    for( plane = 0; plane < depth; plane++ )
    {
        WaitBlit();
        cust->bltcon0 = SRCA | DEST | 0xF0; /* D = A */
        cust->bltcon1 = 0;
        cust->bltapt  = srcPlanes[ plane ] + srcOffset;
        cust->bltdpt  = destPlanes[ plane ] + destOffset;
        cust->bltamod = srcModulo;
        cust->bltdmod = destModulo;
        cust->bltafwm = 0xFFFF;
        cust->bltalwm = 0xFFFF;
        cust->bltsize = size;
    }
    DisownBlitter();
}

/* createMask() - Create Mask */

PLANEPTR createMask( struct BitMap *bitMap )
{
    PLANEPTR mask;
    WORD plane, depth = bitMap->Depth;
    struct Custom *cust = &custom;
    PLANEPTR *planes = bitMap->Planes;
    UWORD bltcon0 = SRCB | DEST | 0xFC;
    UWORD size = blitSize( bitMap->BytesPerRow >> 1, bitMap->Rows );

    if( mask = AllocRaster( bitMap->BytesPerRow << 3, bitMap->Rows ) )
    {
        OwnBlitter();
        for( plane = 0; plane < depth; plane++ )
        {
            WaitBlit();
            cust->bltcon0 = plane == 0 ? bltcon0 : bltcon0 | SRCA;
            cust->bltcon1 = 0;
            cust->bltapt  = mask;
            cust->bltadat = 0;
            cust->bltbpt  = planes[ plane ];
            cust->bltdpt  = mask;
            cust->bltamod = 0;
            cust->bltbmod = 0;
            cust->bltdmod = 0;
            cust->bltsize = size;
        }
        DisownBlitter();
        return( mask );
    }
    return( NULL );
}

VOID freeMask( PLANEPTR mask, struct BitMap *bitMap )
{
    FreeRaster( mask, bitMap->BytesPerRow << 3, bitMap->Rows );
}

/* blitBob() - Blits Blitter Object. */

VOID blitBob( struct BitMap *srcBitMap, struct BitMap *destBitMap, WORD srcX, WORD srcY, WORD destX, WORD destY, WORD width, WORD height, PLANEPTR mask )
{
    WORD plane, depth = destBitMap->Depth;
    struct Custom *cust = &custom;

    PLANEPTR *srcPlanes = srcBitMap->Planes, *destPlanes = destBitMap->Planes;

    WORD srcBPR = srcBitMap->BytesPerRow, destBPR = destBitMap->BytesPerRow;
    UBYTE shift = blitShift( destX );
    UWORD wordWidth = blitWidth( width, shift );
    UWORD lastWordMask = blitMask( width, shift );

    WORD srcModulo = blitModulo( srcBPR, wordWidth );
    WORD destModulo = blitModulo( destBPR, wordWidth );

    LONG srcOffset = blitOffset( srcX, srcY, srcBPR );
    LONG destOffset = blitOffset( destX, destY, destBPR );

    UWORD size = blitSize( wordWidth, height );

    UWORD bltcon0 = SRCB | SRCC | DEST | 0xCA | ( shift << ASHIFTSHIFT );
    UWORD bltcon1 = shift << BSHIFTSHIFT;

    PLANEPTR dest;

    printf("Size = %d %d\n", wordWidth, height );
    printf("Mask = $%X\n", lastWordMask );
    printf("Shift = %d\n", shift );

    if( mask )
    {
        bltcon0 |= SRCA;
    }

    OwnBlitter();
    for( plane = 0; plane < depth; plane++ )
    {
        WaitBlit();
        cust->bltcon0 = bltcon0; /* D = AB + aC */
        cust->bltcon1 = bltcon1;

        if( mask )
        {
            cust->bltapt  = mask + srcOffset;
            cust->bltamod = srcModulo;
        }
        else
        {
            cust->bltadat = 0xFFFF;
        }
        cust->bltbpt  = srcPlanes[ plane ] + srcOffset;
        cust->bltcpt  = dest = destPlanes[ plane ] + destOffset;
        cust->bltdpt  = dest;
        cust->bltbmod = srcModulo;
        cust->bltcmod = destModulo;
        cust->bltdmod = destModulo;
        cust->bltafwm = 0xFFFF;
        cust->bltalwm = lastWordMask;
        cust->bltsize = size;
    }
    DisownBlitter();
}
