
#include <dos/dos.h>
#include <datatypes/pictureclass.h>
#include <libraries/iffparse.h>
#include <exec/memory.h>

#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>

#define RGB( c ) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

#define RowBytes( w ) ((((w)+15)>>4)<<1)

BOOL LoadColors( UBYTE *colors, WORD count, struct ColorMap **cm )
{
    WORD i;

    if( !cm )
        return( TRUE );

    if( !*cm )
        if( !( *cm = GetColorMap( count ) ) )
            return( FALSE );

    for( i = 0; i < count; i++ ) {
        UBYTE red   = colors[ 0 ];
        UBYTE green = colors[ 1 ];
        UBYTE blue  = colors[ 2 ];

        SetRGB32CM( *cm, i, RGB( red ), RGB( green ), RGB( blue ) );
        colors += 3;
    }
    return( TRUE );
}


BOOL UnpackRow( BYTE **bufferptr, LONG *sizeptr, BYTE **planeptr, UBYTE cmp, WORD bpr )
{
    BYTE *buffer = *bufferptr;
    LONG size    = *sizeptr;
    BYTE *plane  = *planeptr;

    if( cmp == cmpNone ) {
        if( size < bpr )
            return( FALSE );

        size -= bpr;
        CopyMem( buffer, plane, bpr );
        buffer += bpr;
        plane  += bpr;
    }
    else if( cmp == cmpByteRun1 ) {

        while( bpr > 0 ) {
            BYTE con;
            if( size < 1 )
                return( FALSE );
            if( ( con = *buffer++ ) >= 0 ) {
                WORD count = con + 1;
                if( size < count || bpr < count )
                    return( FALSE );

                size -= count;
                bpr  -= count;
                while( count-- > 0 )
                    *plane++ = *buffer++;
            }
            else if( con != -128 ) {
                WORD count = ( -con ) + 1;
                BYTE data;
                if( size < 1 || bpr < count )
                    return( FALSE );

                size--;
                bpr -= count;
                data = *buffer++;
                while( count-- > 0 )
                    *plane++ = data;
            }
        }
    }

    *bufferptr = buffer;
    *sizeptr   = size;
    *planeptr  = plane;

    return( TRUE );
}

struct BitMap *ReadBitMap( struct BitMapHeader *bmhd, BYTE *buffer, LONG size )
{
    struct BitMap *bm;
    WORD width  = bmhd->bmh_Width;
    WORD height = bmhd->bmh_Height;
    WORD depth  = bmhd->bmh_Depth;
    UBYTE cmp   = bmhd->bmh_Compression;
    UBYTE msk   = bmhd->bmh_Masking;
    WORD bpr    = RowBytes( width );
    WORD i, j;
    PLANEPTR planes[ 9 ];

    if( cmp != cmpNone && cmp != cmpByteRun1 )
        return( NULL );

    if( msk != mskNone && msk != mskHasTransparentColor )
        return( NULL );

    if( bm = AllocBitMap( width, height, depth, 0, NULL ) ) {
        for( i = 0; i < depth; i++ )
            planes[ i ] = bm->Planes[ i ];

        for( j = 0; j < height; j++ )
            for( i = 0; i < depth; i++ )

                if( !UnpackRow( &buffer, &size, &planes[ i ], cmp, bpr ) ) {
                    FreeBitMap( bm );
                    return( NULL );
                }

        return( bm );
    }
    return( NULL );
}

struct BitMap *ScanPicture( struct IFFHandle *iff, struct ColorMap **cm )
{
    struct BitMap *bm = NULL;

    if( OpenIFF( iff, IFFF_READ ) == 0 ) {

        if( ParseIFF( iff, IFFPARSE_SCAN ) == 0 ) {
            struct StoredProperty *sp;

            if( sp = FindProp( iff, ID_ILBM, ID_BMHD ) ) {
                struct BitMapHeader *bmhd = ( struct BitMapHeader * ) sp->sp_Data;

                if( sp = FindProp( iff, ID_ILBM, ID_CMAP ) ) {
                    UBYTE *colors = sp->sp_Data;
                    WORD count = sp->sp_Size / 3;
                    BOOL owncm = FALSE; /* Own ColorMap provided? */

                    if( cm )
                        if( *cm )
                            owncm = TRUE;

                    if( LoadColors( colors, count, cm ) ) {
                        struct ContextNode *cn;

                        if( cn = CurrentChunk( iff ) ) {
                            BYTE *buffer;
                            LONG size = cn->cn_Size;

                            if( buffer = AllocMem( size, MEMF_PUBLIC ) ) {
                                if( ReadChunkBytes( iff, buffer, size ) == size )
                                    /* Read BODY chunk into BitMap */
                                    bm = ReadBitMap( bmhd, buffer, size );

                                FreeMem( buffer, size );
                            }
                        }
                        if( !bm && owncm )
                            FreeColorMap( *cm );
                    }
                }
            }
        }
        CloseIFF( iff );
    }
    return( bm );
}

struct BitMap *LoadPicture( STRPTR name, struct ColorMap **cm )
{
    struct IFFHandle *iff;
    struct BitMap *bm = NULL;

    if( iff = AllocIFF() ) {
        if( iff->iff_Stream = Open( name, MODE_OLDFILE ) ) {
            InitIFFasDOS( iff );
            if( PropChunk( iff, ID_ILBM, ID_BMHD ) == 0 )
                if( PropChunk( iff, ID_ILBM, ID_CMAP ) == 0 )
                    if( StopChunk( iff, ID_ILBM, ID_BODY ) == 0 )

                        bm = ScanPicture( iff, cm );

            Close( iff->iff_Stream );
        }
        FreeIFF( iff );
    }
    return( bm );
}

void UnloadPicture( struct BitMap *bm, struct ColorMap *cm )
{
    FreeBitMap( bm );
    if( cm )
        FreeColorMap( cm );
}
