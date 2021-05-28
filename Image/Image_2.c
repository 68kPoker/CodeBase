
#include <exec/memory.h>
#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#define WordWidth( w ) (((w) + 15) >> 4)

BOOL cutImage( struct Image *img, struct BitMap *bm, struct Rectangle *clip )
{
    WORD width = clip->MaxX - clip->MinX + 1;
    WORD height = clip->MaxY - clip->MinY + 1;
    UBYTE depth = GetBitMapAttr( bm, BMA_DEPTH );
    struct BitMap aux;
    WORD i;
    WORD planeSize = WordWidth( width ) * sizeof( UWORD ) * height;

    if( img->ImageData = AllocVec( planeSize * depth, MEMF_CHIP ) )
    {
        img->LeftEdge = 0;
        img->TopEdge = 0;
        img->Width = width;
        img->Height = height;
        img->Depth = depth;
        img->PlanePick = 0xff;
        img->PlaneOnOff = 0x00;
        img->NextImage = NULL;

        InitBitMap( &aux, depth, width, height );
        aux.Planes[ 0 ] = ( PLANEPTR ) img->ImageData;
        for( i = 1; i < depth; i++ )
        {
            aux.Planes[ i ] = aux.Planes[ i - 1 ] + planeSize;
        }

        BltBitMap( bm, clip->MinX, clip->MinY, &aux, 0, 0, width, height, 0xc0, 0xff, NULL );
        return( TRUE );
    }
    return( FALSE );
}
