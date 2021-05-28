
#include "Picture.h"

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <exec/memory.h>
#include <datatypes/pictureclass.h>

#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#define MIN(a,b) ((a)<=(b)?(a):(b))

#define RowBytes(w) ((((w)+15)>>4)<<1)
#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

struct BufferInfo
    {
    BYTE *buffer, *current;
    LONG bytesLeft, size;
    };

LONG readChunkBytes(struct IFFHandle *iff, struct BufferInfo *bi, BYTE *dest, LONG bytes)
{
    LONG sum = 0, count;
    while (bytes > 0)
        {
        if (bi->bytesLeft == 0)
            {
            bi->current = bi->buffer;
            if ((bi->bytesLeft = ReadChunkBytes(iff, bi->current, bi->size)) == 0)
                {
                return(sum);
                }
            }

        count = MIN(bytes, bi->bytesLeft);

        CopyMem(bi->current, dest, count);
        bi->current += count;
        dest += count;
        bi->bytesLeft -= count;
        bytes -= count;
        sum += count;
        }
    return(sum);
}

LONG unpackRow(struct IFFHandle *iff, struct BufferInfo *bi, BYTE *dest, WORD bpr, UBYTE cmp)
{
    if (cmp == cmpNone)
        {
        if (readChunkBytes(iff, bi, dest, bpr) != bpr)
            {
            return(CMPERR);
            }
        }
    else if (cmp == cmpByteRun1)
        {
        while (bpr > 0)
            {
            BYTE con;
            if (readChunkBytes(iff, bi, &con, 1) != 1)
                {
                return(CMPERR);
                }
            if (con >= 0)
                {
                WORD count = con + 1;
                if (bpr < count || readChunkBytes(iff, bi, dest, count) != count)
                    {
                    return(CMPERR);
                    }
                dest += count;
                bpr -= count;
                }
            else if (con != -128)
                {
                WORD count = (-con) + 1;
                BYTE data;
                if (bpr < count || readChunkBytes(iff, bi, &data, 1) != 1)
                    {
                    return(CMPERR);
                    }
                bpr -= count;
                while (count-- > 0)
                    {
                    *dest++ = data;
                    }
                }
            }
        }
    return(0);
}

LONG loadColors(struct StoredProperty *sp, struct PictureInfo *pi)
{
    WORD colors = sp->sp_Size / 3;
    LONG size = (colors * 3) + 2;

    if (pi->colors = AllocVec(size * sizeof(ULONG), MEMF_PUBLIC))
        {
        UBYTE *cmap = sp->sp_Data;
        WORD color;

        pi->colors[0] = colors << 16;
        for (color = 0; color < colors; color++)
            {
            UBYTE red = *cmap++;
            UBYTE green = *cmap++;
            UBYTE blue = *cmap++;
            pi->colors[(color * 3) + 1] = RGB(red);
            pi->colors[(color * 3) + 2] = RGB(green);
            pi->colors[(color * 3) + 3] = RGB(blue);
            }
        pi->colors[(colors * 3) + 1] = 0L;
        return(0);
        }
    return(NOMEM);
}

VOID freeColors(struct PictureInfo *pi)
{
    FreeVec(pi->colors);
}

LONG unpackPicture(struct IFFHandle *iff, struct PictureInfo *pi)
{
    struct StoredProperty *sp;
    LONG err;

    if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
        {
        struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;
        if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
            {
            if ((err = loadColors(sp, pi)) == 0)
                {
                struct BufferInfo bi;
                const LONG bufSize = 4096;
                bi.size = bufSize;
                bi.bytesLeft = 0;
                if (bi.buffer = bi.current = AllocMem(bufSize, MEMF_PUBLIC))
                    {
                    struct BitMap *bm;
                    UWORD width = bmhd->bmh_Width, height = bmhd->bmh_Height;
                    UBYTE depth = bmhd->bmh_Depth, cmp = bmhd->bmh_Compression, masking = bmhd->bmh_Masking;
                    UWORD bpr = RowBytes(width);

                    if (masking == mskHasMask)
                        {
                        depth++;
                        }

                    if (pi->bitmap = bm = AllocBitMap(width, height, depth, 0, NULL))
                        {
                        PLANEPTR planes[9];
                        WORD row, plane;

                        for (plane = 0; plane < depth; plane++)
                            {
                            planes[plane] = bm->Planes[plane];
                            }

                        for (row = 0; row < height; row++)
                            {
                            for (plane = 0; plane < depth; plane++)
                                {
                                if (err = unpackRow(iff, &bi, planes[plane], bpr, cmp))
                                    {
                                    break;
                                    }
                                planes[plane] += bpr;
                                }
                            }
                        if (!err)
                            {
                            FreeMem(bi.buffer, bi.size);
                            return(0);
                            }
                        FreeBitMap(bm);
                        }
                    else
                        err = NOGFXMEM;
                    FreeMem(bi.buffer, bi.size);
                    }
                else
                    err = NOMEM;
                freeColors(pi);
                }
            }
        else
            err = NOCMAP;
        }
    else
        err = NOBMHD;

    return(err);
}

LONG loadPicture(STRPTR name, struct PictureInfo *pi)
{
    struct IFFHandle *iff;
    LONG err;
    LONG props[] =
        {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP
        };
    LONG propCount = 2;

    if (iff = AllocIFF())
        {
        if (iff->iff_Stream = Open(name, MODE_OLDFILE))
            {
            InitIFFasDOS(iff);
            if ((err = OpenIFF(iff, IFFF_READ)) == 0)
                {
                if ((err = PropChunks(iff, props, propCount)) == 0)
                    {
                    if ((err = StopChunk(iff, ID_ILBM, ID_BODY)) == 0)
                        {
                        if ((err = ParseIFF(iff, IFFPARSE_SCAN)) == 0)
                            {
                            err = unpackPicture(iff, pi);
                            }
                        }
                    }
                CloseIFF(iff);
                }
            Close(iff->iff_Stream);
            }
        else
            err = NOFILE;
        FreeIFF(iff);
        }
    else
        err = NOMEM;
    return(err);
}

VOID freePicture(struct PictureInfo *pi)
{
    FreeBitMap(pi->bitmap);
    freeColors(pi);
}
