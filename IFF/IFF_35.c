
/* Basic functions */

#include <stdio.h>
#include "debug.h"

#include <exec/types.h>
#include <exec/memory.h>
#include <graphics/gfx.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>

#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "Basic.h"

#define BUF_SIZE 2048
#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

struct buffer
{
    struct IFFHandle *iff;
    BYTE *beg, *cur;
    ULONG size, left;
};

/* Count ID pairs in null-terminated array */

UWORD countIDs(ULONG *ids)
{
    UWORD count = 0;

    if (ids == NULL)
    {
        return(0);
    }

    while (*ids++)
    {
        count++;
    }

    return(count / 2);
}

/* Repeat a byte N-times */

VOID repeatByte(BYTE *buf, BYTE byte, UWORD n)
{
    while (n-- > 0)
    {
        *buf++ = byte;
    }
}

/* Buffered chunk reading */

BOOL initBuffer(struct buffer *buf, struct IFFHandle *iff)
{
    buf->iff = iff;
    if (buf->beg = AllocMem(buf->size = BUF_SIZE, MEMF_PUBLIC))
    {
        buf->cur = buf->beg;
        if ((buf->left = ReadChunkBytes(iff, buf->beg, buf->size)) > 0)
        {
            return(TRUE);
        }
        FreeMem(buf->beg, buf->size);
    }
    return(FALSE);
}

VOID freeBuffer(struct buffer *buf)
{
    FreeMem(buf->beg, buf->size);
}

ULONG readBuffer(struct buffer *buf, BYTE *dest, ULONG bytes)
{
    ULONG actual = 0, min;

    while (bytes > 0)
    {
        if (buf->left == 0)
        {
            if ((buf->left = ReadChunkBytes(buf->iff, buf->beg, buf->size)) == 0)
            {
                break;
            }
            buf->cur = buf->beg;
        }

        if (bytes < buf->left)
        {
            min = bytes;
        }
        else
        {
            min = buf->left;
        }

        CopyMem(buf->cur, dest, min);

        buf->cur += min;
        dest += min;
        actual += min;

        bytes -= min;
        buf->left -= min;
    }
    return(actual);
}

/* ILBM BODY decompression */

BOOL unpackRow(struct buffer *buf, BYTE *dest, UWORD bpr, UBYTE cmp)
{
    if (cmp == cmpNone)
    {
        if (readBuffer(buf, dest, bpr) != bpr)
        {
            return(FALSE);
        }
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE con;
            if (readBuffer(buf, &con, 1) != 1)
            {
                return(FALSE);
            }
            if (con >= 0)
            {
                WORD count = con + 1;
                if (bpr < count || readBuffer(buf, dest, count) != count)
                {
                    return(FALSE);
                }
                bpr -= count;
                dest += count;
            }
            else if (con != -128)
            {
                WORD count = (-con) + 1;
                BYTE data;
                if (bpr < count || readBuffer(buf, &data, 1) != 1)
                {
                    return(FALSE);
                }
                bpr -= count;
                repeatByte(dest, data, count);
                dest += count;
            }
        }
    }
    else
    {
        return(FALSE);
    }

    return(TRUE);
}

BOOL unpackILBM(struct IFFHandle *iff, struct BitMap *bm, UBYTE cmp)
{
    struct buffer buf;
    BOOL success = FALSE;
    UWORD bpr = RowBytes(GetBitMapAttr(bm, BMA_WIDTH));

    if (initBuffer(&buf, iff))
    {
        PLANEPTR planes[9];
        WORD plane, row;

        for (plane = 0; plane < bm->Depth; plane++)
        {
            planes[plane] = bm->Planes[plane];
        }

        for (row = 0; row < bm->Rows; row++)
        {
            for (plane = 0; plane < bm->Depth; plane++)
            {
                if (!(success = unpackRow(&buf, planes[plane], bpr, cmp)))
                {
                    break;
                }
                planes[plane] += bm->BytesPerRow;
            }
            if (!success)
            {
                break;
            }
        }

        freeBuffer(&buf);
    }
    return(success);
}

struct IFFHandle *openIFF(STRPTR name, LONG mode)
{
    struct IFFHandle *iff;
    LONG dosmodes[] = { MODE_OLDFILE, MODE_NEWFILE };

    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = Open(name, dosmodes[mode & 1]))
        {
            InitIFFasDOS(iff);
            if (!OpenIFF(iff, mode))
            {
                return(iff);
            }
            Close(iff->iff_Stream);
        }
        FreeIFF(iff);
    }
    return(NULL);
}

void closeIFF(struct IFFHandle *iff)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}

BOOL scanIFF(struct IFFHandle *iff, ULONG *props, ULONG *stops)
{
    if (!PropChunks(iff, props, countIDs(props)))
    {
        if (!StopChunks(iff, stops, countIDs(stops)))
        {
            if (!ParseIFF(iff, IFFPARSE_SCAN))
            {
                return(TRUE);
            }
        }
    }
    return(FALSE);
}

BOOL loadCMAP(struct IFFHandle *iff, struct ColorMap *cm)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        UBYTE *cmap = sp->sp_Data;
        LONG size = sp->sp_Size;
        WORD i, colors = size / 3;

        for (i = 0; i < colors; i++)
        {
            UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;
            SetRGB32CM(cm, i, RGB(red), RGB(green), RGB(blue));
        }
        return(TRUE);
    }
    return(FALSE);
}

struct BitMap *loadILBM(STRPTR name, struct ColorMap *cm)
{
    struct IFFHandle *iff;
    struct StoredProperty *sp;
    struct BitMapHeader *bmhd;
    struct BitMap *bm = NULL;
    ULONG props[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP,
        0
    };

    ULONG stops[] =
    {
        ID_ILBM, ID_BODY,
        0
    };

    if (iff = openIFF(name, IFFF_READ))
    {
        if (scanIFF(iff, props, stops))
        {
            if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
            {
                bmhd = (struct BitMapHeader *)sp->sp_Data;
                if (bm = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, BMF_INTERLEAVED, NULL))
                {
                    if ((!loadCMAP(iff, cm)) || (!unpackILBM(iff, bm, bmhd->bmh_Compression)))
                    {
                        FreeBitMap(bm);
                        bm = NULL;
                    }
                }
            }
        }
        closeIFF(iff);
    }
    return(bm);
}
