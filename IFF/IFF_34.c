
#include <stdio.h>

#include <dos/dos.h>
#include <libraries/iffparse.h>

#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>

#include "CIFF.h"

ULONG handleBMHD( struct Hook *hook, struct CBMHD *bmhd, LONG *cmd )
{
    struct IFFHandle *iff = ( struct IFFHandle * ) hook->h_Data;

    if( ReadChunkBytes( iff, &bmhd->bmhd, sizeof( struct BitMapHeader ) ) == sizeof( struct BitMapHeader ) ) {
        UBYTE cmp = bmhd->bmhd.bmh_Compression;
        UBYTE msk = bmhd->bmhd.bmh_Masking;

        if( cmp != cmpNone && cmp != cmpByteRun1 )
            return( UNKNOWN_CMP );

        if( msk != mskNone && msk != mskHasTransparentColor )
            return( UNKNOWN_MSK );

        return( 0 );
    }

    return( NO_DATA );
}

ULONG handleCMAP( struct Hook *hook, struct CCMAP *cmap, LONG *cmd )
{
    struct IFFHandle *iff = ( struct IFFHandle * ) hook->h_Data;
    struct ContextNode *cn;
    LONG err;

    if( cn = CurrentChunk( iff ) ) {
        LONG size = cn->cn_Size;
        WORD colors = size / 3;
        struct ColorMap *cm;

        if( cm = GetColorMap( colors ) ) {

            WORD i;

            for( i = 0; i < colors; i++ ) {
                UBYTE triplet[ 3 ];

                if( ReadChunkBytes( iff, triplet, sizeof( triplet ) ) == sizeof( triplet ) ) {
                    UBYTE red = triplet[ 0 ];
                    UBYTE green = triplet[ 1 ];
                    UBYTE blue = triplet[ 2 ];

                    SetRGB32CM( cm, i, RGB( red ), RGB( green ), RGB( blue ) );
                }
                else {
                    err = NO_DATA;
                    break;
                }
            }

            if( i == colors ) {
                cmap->cm = cm;
                return( 0 );
            }

            FreeColorMap( cm );
        }
        else
            err = NO_MEM;
    }
    else
        err = NO_CN;
    return( err );
}

ULONG handleILBM( struct Hook *hook, struct CILBM *ilbm, LONG *cmd )
{
    struct IFFHandle *iff = ( struct IFFHandle * ) hook->h_Data;
    WORD width = ilbm->bmhd.bmhd.bmh_Width;
    WORD height = ilbm->bmhd.bmhd.bmh_Height;
    UBYTE depth = ilbm->bmhd.bmhd.bmh_Depth;
    UBYTE cmp = ilbm->bmhd.bmhd.bmh_Compression;
    struct BitMap *bm;
    LONG err = 0;

    if( bm = AllocBitMap( width, height, depth, 0, NULL ) ) {
        PLANEPTR planes[ 9 ];
        WORD i, j, bpr = RowBytes( width );

        for( i = 0; i < depth; i++ )
            planes[ i ] = bm->Planes[ i ];

        for( j = 0; j < height; j++ ) {
            for( i = 0; i < depth; i++ ) {
                if( cmp == cmpNone ) {
                    if( ReadChunkBytes( iff, planes[ i ], bpr ) != bpr ) {
                        err = NO_DATA;
                        break;
                    }
                    planes[ i ] += bm->BytesPerRow;
                }
                else if( cmp == cmpByteRun1 ) {
                    WORD left = bpr;

                    while( left > 0 ) {
                        BYTE c;

                        if( ReadChunkBytes( iff, &c, 1 ) != 1 ) {
                            err = NO_DATA;
                            break;
                        }
                        if( c >= 0 ) {
                            WORD count = c + 1;
                            if( left < count || ReadChunkBytes( iff, planes[ i ], count ) != count ) {
                                err = NO_DATA;
                                break;
                            }
                            left -= count;
                            planes[ i ] += count;
                        }
                        else if( c != -128 ) {
                            WORD count = ( -c ) + 1;
                            BYTE data;
                            if( left < count || ReadChunkBytes( iff, &data, 1 ) != 1 ) {
                                err = NO_DATA;
                                break;
                            }
                            left -= count;
                            while( count-- > 0 )
                                *planes[ i ]++ = data;
                        }
                    }
                    if( err )
                        break;
                }
            }
            if( err )
                break;
        }
        if( !err ) {
            ilbm->bm = bm;
            return( IFF_RETURN2CLIENT );
        }
        FreeBitMap( bm );
    }
    else
        err = NO_GFXMEM;
    return( err );
}

LONG loadIFF( STRPTR name, struct CChunk *en, struct CChunk *ex )
{
    struct IFFHandle *iff;
    LONG err = 0;

    if( iff = AllocIFF() ) {
        if( iff->iff_Stream = Open( name, MODE_OLDFILE ) ) {
            InitIFFasDOS( iff );
            if( ( err = OpenIFF( iff, IFFF_READ ) ) == 0 ) {
                while( en && !err ) {
                    en->hook.h_Data = ( APTR ) iff;
                    err = EntryHandler( iff, en->type, en->id, IFFSLI_ROOT, &en->hook, en->user );
                    en = en->next;
                }
                while( ex && !err ) {
                    ex->hook.h_Data = ( APTR ) iff;
                    err = ExitHandler( iff, ex->type, ex->id, IFFSLI_ROOT, &ex->hook, ex->user );
                    ex = ex->next;
                }
                if( !err )
                    err = ParseIFF( iff, IFFPARSE_SCAN );
                CloseIFF( iff );
            }
            Close( iff->iff_Stream );
        }
        else
            err = NO_FILE;
        FreeIFF( iff );
    }
    else
        err = NO_MEM;
    return( err );
}

void initChunk( struct CChunk *chunk, struct CChunk *prev, ULONG type, ULONG id, HOOKFUNC func, APTR user )
{
    chunk->type = type;
    chunk->id = id;
    chunk->hook.h_Entry = HookEntry;
    chunk->hook.h_SubEntry = func;
    chunk->user = user;
    chunk->next = NULL;

    if( prev )
        prev->next = chunk;
}

LONG loadILBM( STRPTR name, struct CILBM *ilbm )
{
    struct CChunk chunk[ 3 ] = { 0 };

    initChunk( chunk + 0, NULL, ID_ILBM, ID_BMHD, handleBMHD, &ilbm->bmhd );
    initChunk( chunk + 1, chunk + 0, ID_ILBM, ID_CMAP, handleCMAP, &ilbm->cmap );
    initChunk( chunk + 2, chunk + 1, ID_ILBM, ID_BODY, handleILBM, ilbm );

    LONG err = loadIFF( name, &chunk[ 0 ], NULL );

    return( err );
}

void unloadILBM( struct CILBM *ilbm )
{
    FreeColorMap( ilbm->cmap.cm );
    FreeBitMap( ilbm->bm );
}
