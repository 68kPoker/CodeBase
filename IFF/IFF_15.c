
/* RCS Header */

/* $Id$ */

/* Header files */

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <exec/memory.h>

/* Proto files */

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>

/* Macros */

#define RGB(val) ((val)|((val)<<8)|((val)<<16)|((val)<<24))

#define RowBytes(width) ((((width)+15)>>4)<<1)

/* Structs */

struct IFFBuffer
{
    struct IFFHandle *iff;
    BYTE *bufBegin, *bufCurrent;
    LONG bufSize, bufLeft;
};

/* Functions */

/* IFF Scanner */

WORD countChunks(ULONG *chunks)
{
    WORD count = 0;
    if (!chunks)
    {
        return(0);
    }
    while (*chunks++)
    {
        count++;
    }
    return(count / 2);
}

LONG scanIFF(struct IFFHandle *iff, ULONG *props, ULONG *stops)
{
    LONG err;

    if ((err = OpenIFF(iff, IFFF_READ)) == 0)
    {
        if ((err = PropChunks(iff, props, countChunks(props))) == 0)
        {
            if ((err = StopChunks(iff, stops, countChunks(stops))) == 0)
            {
                err = ParseIFF(iff, IFFPARSE_SCAN);
                if (err == 0 || err == IFFERR_EOC || err == IFFERR_EOF)
                {
                    return(err);
                }
            }
        }
        CloseIFF(iff);
    }
    return(err);
}

/* ILBM interpreter */

LONG readChunkBytes(struct IFFBuffer *buf, BYTE *dest, WORD len)
{
    LONG actual = 0;

    if (!buf->bufBegin)
    {
        if ((buf->bufBegin = AllocMem(buf->bufSize = 4096, MEMF_PUBLIC)) == NULL)
        {
            return(0);
        }
    }

    while (len > 0)
    {
        WORD min;
        if (buf->bufLeft == 0)
        {
            if ((buf->bufLeft = ReadChunkBytes(buf->iff, buf->bufBegin, buf->bufSize)) == 0)
            {
                break;
            }
            buf->bufCurrent = buf->bufBegin;
        }

        if (len < buf->bufLeft)
        {
            min = len;
        }
        else
        {
            min = buf->bufLeft;
        }
        CopyMem(buf->bufCurrent, dest, min);
        buf->bufCurrent += min;
        dest += min;
        len -= min;
        buf->bufLeft -= min;
        actual += min;
    }
    return(actual);
}

BOOL unpackRow(struct IFFBuffer *buf, PLANEPTR plane, WORD bpr, UBYTE cmp)
{
    if (cmp == cmpNone)
    {
        return(readChunkBytes(buf, plane, bpr) == bpr);
    }

    while (bpr > 0)
    {
        BYTE c;
        if (readChunkBytes(buf, &c, 1) != 1)
        {
            return(FALSE);
        }
        if (c >= 0)
        {
            WORD count = c + 1;
            if (bpr < count || readChunkBytes(buf, plane, count) != count)
            {
                return(FALSE);
            }
            plane += count;
            bpr -= count;
        }
        else if (c != -128)
        {
            WORD count = (-c) + 1;
            BYTE data;
            if (bpr < count || readChunkBytes(buf, &data, 1) != 1)
            {
                return(FALSE);
            }
            bpr -= count;
            while (count-- > 0)
            {
                *plane++ = data;
            }
        }
    }
    return(TRUE);
}

struct BitMap *interpretILBM(struct IFFHandle *iff, ULONG **saveColors)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
    {
        struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;
        if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
        {
            UBYTE *palData = sp->sp_Data;
            LONG  palSize  = sp->sp_Size;

            UBYTE cmp = bmhd->bmh_Compression;
            UBYTE msk = bmhd->bmh_Masking;

            if ((cmp == cmpNone || cmp == cmpByteRun1) &&
                (msk == mskNone || msk == mskHasTransparentColor))
            {
                WORD width  = bmhd->bmh_Width;
                WORD height = bmhd->bmh_Height;
                UBYTE depth = bmhd->bmh_Compression;
                WORD bpr    = RowBytes(width);

                struct BitMap *bm;

                if (bm = AllocBitMap(width, height, depth, BMF_INTERLEAVED, NULL))
                {
                    ULONG *colors;

                    if (colors = AllocVec((palSize + 2) * sizeof(ULONG), MEMF_PUBLIC))
                    {
                        WORD c, i, j;
                        PLANEPTR planes[9];
                        struct IFFBuffer buf = { iff };
                        BOOL success = FALSE;

                        colors[0] = (palSize / 3) << 16;
                        colors[palSize + 1] = 0;
                        for (c = 0; c < palSize; c++)
                        {
                            UBYTE val = *palData++;
                            colors[c + 1] = RGB(val);
                        }

                        for (i = 0; i < depth; i++)
                        {
                            planes[i] = bm->Planes[i];
                        }

                        for (j = 0; j < height; j++)
                        {
                            for (i = 0; i < depth; i++)
                            {
                                if (!(success = unpackRow(&buf, planes[i], bpr, cmp)))
                                {
                                    break;
                                }
                                planes[i] += bm->BytesPerRow;
                            }
                            if (!success)
                            {
                                break;
                            }
                        }

                        if (buf.bufBegin)
                        {
                            FreeMem(buf.bufBegin, buf.bufSize);
                        }
                        if (success)
                        {
                            *saveColors = colors;
                            return(bm);
                        }
                        FreeVec(colors);
                    }
                    FreeBitMap(bm);
                }
            }
        }
    }
    return(NULL);
}

struct BitMap *loadILBM(STRPTR name, ULONG **saveColors)
{
    struct IFFHandle *iff;
    struct BitMap *bm = NULL;
    ULONG props[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP,
        0
    }, stops[] =
    {
        ID_ILBM, ID_BODY,
        0
    };

    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = Open(name, MODE_OLDFILE))
        {
            InitIFFasDOS(iff);
            if (scanIFF(iff, props, stops) == 0)
            {
                bm = interpretILBM(iff, saveColors);
                CloseIFF(iff);
            }
            Close(iff->iff_Stream);
        }
        FreeIFF(iff);
    }
    return(bm);
}
