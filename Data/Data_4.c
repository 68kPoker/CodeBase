
#include <datatypes/pictureclass.h>
#include <clib/datatypes_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>

#include "Data.h"
#include "Blitter.h"

/* loadBitMap() - Load BitMap from file */

struct BitMap *loadBitMap( struct Screen *s, STRPTR name )
{
    Object *o;

    if( o = NewDTObject( name,
        DTA_GroupID,    GID_PICTURE,
        PDTA_Screen,    s,
        PDTA_Remap,     FALSE,
        TAG_DONE ) )
    {
        struct BitMap *bm, *cpy;
        ULONG *cregs, numColors;
        WORD col;
        struct ColorMap *cm = s->ViewPort.ColorMap;
        struct BitMapHeader *bmhd;

        DoDTMethod( o, NULL, NULL, DTM_PROCLAYOUT, NULL, TRUE );
        GetDTAttrs( o,
            PDTA_BitMapHeader,  &bmhd,
            PDTA_BitMap,    &bm,
            PDTA_CRegs,     &cregs,
            PDTA_NumColors, &numColors,
            TAG_DONE );

        for( col = 0; col < numColors; col++ )
        {
            SetRGB32CM( cm, col, cregs[ 0 ], cregs[ 1 ], cregs[ 2 ] );
            cregs += 3;
        }
        MakeScreen( s );
        RethinkDisplay();
        if( cpy = AllocBitMap( bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, 0, NULL ) )
        {
            blitIcon( bm, cpy, 0, 0, 0, 0, bmhd->bmh_Width, bmhd->bmh_Height );
            WaitBlit();
            DisposeDTObject( o );
            return( cpy );
        }
        DisposeDTObject( o );
    }
    return( NULL );
}
