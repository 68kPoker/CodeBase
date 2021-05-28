
/* $Id$ */

#include <libraries/iffparse.h>
#include <exec/memory.h>
#include <datatypes/pictureclass.h>

#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include "IFF.h"
#include "IFF_protos.h"

BOOL openIFF( struct IFFHandle *iff, ULONG s, BOOL dos, LONG mode )
{
    LONG err;

    iff->iff_Stream = s;

    if( dos )
    {
        InitIFFasDOS( iff );
    }
    else
    {
        InitIFFasClip( iff );
    }

    if( ( err = OpenIFF( iff, mode ) ) == 0 )
    {
        return( TRUE );
    }
    return( FALSE );
}

BOOL readPalette( struct ILBMInfo *ii, struct StoredProperty *sp )
{
    WORD size = sp->sp_Size;
    ULONG *cur;

    if( ii->palette = cur = AllocVec( ( size + 2 ) * sizeof( ULONG ), MEMF_PUBLIC ) )
    {
        WORD i;
        UBYTE *cmap = sp->sp_Data;

        *cur++ = ( size / 3 ) << 16;
        for( i = 0; i < size; i++ )
        {
            UBYTE data = *cmap++;
            *cur++ = RGB( data );
        }
        *cur = 0L;
        return( TRUE );
    }
    return( FALSE );
}

LONG readChunkBytes( struct IFFHandle *iff, struct bodyBuffer *bb, BYTE *dest, WORD bytes )
{
    LONG amount = 0, min;

    while( bytes > 0 )
    {
        if( bb->left == 0 )
        {
            if( ( bb->left = ReadChunkBytes( iff, bb->beg, bb->size ) ) == 0 )
            {
                return( amount );
            }
            bb->cur = bb->beg;
        }
        if( bytes < bb->left )
        {
            min = bytes;
        }
        else
        {
            min = bb->left;
        }
        CopyMem( bb->cur, dest, min );
        bb->cur += min;
        dest += min;
        amount += min;
        bytes -= min;
        bb->left -= min;
    }
    return( amount );
}

BOOL unpackRow( struct IFFHandle *iff, struct bodyBuffer *bb, BYTE *dest, WORD bpr, UBYTE cmp )
{
    if( cmp == cmpNone )
    {
        if( readChunkBytes( iff, bb, dest, bpr ) != bpr )
        {
            return( FALSE );
        }
    }
    else if( cmp == cmpByteRun1 )
    {
        while( bpr > 0 )
        {
            BYTE c;

            if( readChunkBytes( iff, bb, &c, 1 ) != 1 )
            {
                return( FALSE );
            }
            if( c >= 0 )
            {
                WORD count = c + 1;
                if( bpr < count || readChunkBytes( iff, bb, dest, count ) != count )
                {
                    return( FALSE );
                }
                bpr -= count;
                dest += count;
            }
            else if( c != -128 )
            {
                WORD count = ( -c ) + 1;
                BYTE data;
                if( bpr < count || readChunkBytes( iff, bb, &data, 1 ) != 1 )
                {
                    return( FALSE );
                }
                bpr -= count;
                while( count-- > 0 )
                {
                    *dest++ = data;
                }
            }
        }
    }
    else
    {
        return( FALSE );
    }
    return( TRUE );
}

BOOL readBitMap( struct ILBMInfo *ii, struct IFFHandle *iff )
{
    struct BitMapHeader *bmhd = ii->bmhd;
    struct bodyBuffer bb;
    WORD i, j;
    BYTE *curPlane;
    WORD bpr;
    BOOL success;

    if( ii->brush = AllocBitMap( bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, BMF_INTERLEAVED | BMF_CLEAR, NULL ) )
    {
        if( bb.beg = AllocMem( bb.size = BODY_BUFFER_SIZE, MEMF_PUBLIC ) )
        {
            bb.left = 0;
            curPlane = ii->brush->Planes[ 0 ];
            bpr = RowBytes( bmhd->bmh_Width );

            for( i = 0; i < bmhd->bmh_Height; i++ )
            {
                for( j = 0; j < bmhd->bmh_Depth; j++ )
                {
                    if ( !( success = unpackRow( iff, &bb, curPlane, bpr, bmhd->bmh_Compression ) ) )
                    {
                        break;
                    }
                    curPlane += bpr;
                }
                if( !success )
                {
                    break;
                }
            }
            FreeMem( bb.beg, bb.size );
            if( success )
            {
                return( TRUE );
            }
        }
        FreeBitMap( ii->brush );
    }
    return( FALSE );
}

BOOL readILBM( struct ILBMInfo *ii, struct IFFHandle *iff )
{
    LONG err;

    if( ( err = PropChunk( iff, ID_ILBM, ID_BMHD ) ) == 0 &&
        ( err = PropChunk( iff, ID_ILBM, ID_CMAP ) ) == 0 &&
        ( err = StopChunk( iff, ID_ILBM, ID_BODY ) ) == 0 )
    {
        if( ( err = ParseIFF( iff, IFFPARSE_SCAN ) ) == 0 )
        {
            struct StoredProperty *sp;

            if( sp = FindProp( iff, ID_ILBM, ID_BMHD ) )
            {
                ii->bmhd = ( struct BitMapHeader * ) sp->sp_Data;
                if( sp = FindProp( iff, ID_ILBM, ID_CMAP ) )
                {
                    if( readPalette( ii, sp ) )
                    {
                        if( readBitMap( ii, iff ) )
                        {
                            return( TRUE );
                        }
                        FreeVec( ii->palette );
                    }
                }
            }
        }
    }
    return( FALSE );
}
