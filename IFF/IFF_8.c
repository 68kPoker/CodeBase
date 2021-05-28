
/* $Log$ */

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <exec/memory.h>

#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "IFF.h"

struct IFFHandle *openIFF(STRPTR name)
{
    struct IFFHandle *iff;

    if (iff = AllocIFF())
    {
        BPTR f;

        if (iff->iff_Stream = f = Open(name, MODE_OLDFILE))
        {
            InitIFFasDOS(iff);
            if (OpenIFF(iff, IFFF_READ) == 0)
            {
                return(iff);
            }
            Close(f);
        }
        FreeIFF(iff);
    }
    return(NULL);
}

BOOL parseIFF(struct IFFHandle *iff, ULONG *props, WORD propsCount, ULONG *stops, ULONG stopsCount)
{
    if (PropChunks(iff, props, propsCount) == 0)
    {
        if (StopChunks(iff, stops, stopsCount) == 0)
        {
            if (ParseIFF(iff, IFFPARSE_SCAN) == 0)
            {
                return(TRUE);
            }
        }
    }
    return(FALSE);
}

UBYTE *findProp(struct IFFHandle *iff, ULONG type, ULONG id)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, type, id))
    {
        return(sp->sp_Data);
    }
    return(NULL);
}

VOID closeIFF(struct IFFHandle *iff)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}

struct IFFHandle *openBitMap(STRPTR name, struct BitMapHeader **bmhd)
{
    struct IFFHandle *iff;
    ULONG props[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP
    };

    ULONG stops[] =
    {
        ID_ILBM, ID_BODY
    };

    if (iff = openIFF(name))
    {
        if (parseIFF(iff, props, 2, stops, 1))
        {
            if (*bmhd = (struct BitMapHeader *)findProp(iff, ID_ILBM, ID_BMHD))
            {
                return(iff);
            }
        }
        closeIFF(iff);
    }
    return(NULL);
}

BOOL readColorMap(struct IFFHandle *iff, struct ColorMap **cm)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        WORD colors = sp->sp_Size / 3;

        if (*cm = GetColorMap(colors))
        {
            UBYTE *cmap = sp->sp_Data;
            WORD i;

            for (i = 0; i < colors; i++)
            {
                UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;

                SetRGB32CM(*cm, i, RGB(red), RGB(green), RGB(blue));
            }
            return(TRUE);
        }
    }
    return(FALSE);
}

BOOL unpackRow(BYTE **srcptr, LONG *sizeptr, BYTE **destptr, WORD bpr, UBYTE cmp)
{
    BYTE *src = *srcptr;
    LONG size = *sizeptr;
    BYTE *dest = *destptr;

    if (cmp == cmpNone)
    {
        if (size < bpr)
        {
            return(FALSE);
        }
        size -= bpr;
        CopyMem(src, dest, bpr);
        src += bpr;
        dest += bpr;
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE con;
            if (size < 1)
            {
                return(FALSE);
            }
            size--;
            if ((con = *src++) >= 0)
            {
                WORD count = con + 1;
                if (size < count || bpr < count)
                {
                    return(FALSE);
                }
                size -= count;
                bpr -= count;
                while (count-- > 0)
                {
                    *dest++ = *src++;
                }
            }
            else if (con != -128)
            {
                WORD count = (-con) + 1;
                BYTE byte;
                if (size < 1 || bpr < count)
                {
                    return(FALSE);
                }
                size--;
                bpr -= count;
                byte = *src++;
                while (count-- > 0)
                {
                    *dest++ = byte;
                }
            }
        }
    }
    else
    {
        return(FALSE);
    }

    *srcptr = src;
    *sizeptr = size;
    *destptr = dest;
    return(TRUE);
}

BOOL unpackBitMap(struct IFFHandle *iff, struct BitMap *bm, struct BitMapHeader *bmhd)
{
    struct ContextNode *cn;
    BOOL success = FALSE;

    if (cn = CurrentChunk(iff))
    {
        LONG size = cn->cn_Size;
        BYTE *buffer;

        if (buffer = AllocMem(size, MEMF_PUBLIC))
        {
            if (ReadChunkBytes(iff, buffer, size) == size)
            {
                WORD  i, j;
                WORD  depth  = bmhd->bmh_Depth;
                WORD  height = bmhd->bmh_Height;
                UBYTE cmp    = bmhd->bmh_Compression;
                WORD  width  = bmhd->bmh_Width;
                WORD  bpr    = RowBytes(width);
                PLANEPTR planes[8];
                BYTE *current = buffer;

                for (i = 0; i < depth; i++)
                {
                    planes[i] = bm->Planes[i];
                }

                for (j = 0; j < height; j++)
                {
                    for (i = 0; i < depth; i++)
                    {
                        if (!(success = unpackRow(&current, &size, &planes[i], bpr, cmp)))
                        {
                            break;
                        }
                    }
                    if (!success)
                    {
                        break;
                    }
                }
            }
            FreeMem(buffer, cn->cn_Size);
        }
    }
    return(success);
}

struct BitMap *readBitMap(struct IFFHandle *iff, struct BitMapHeader *bmhd)
{
    struct BitMap *bm;

    if (bm = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, 0, NULL))
    {
        if (unpackBitMap(iff, bm, bmhd))
        {
            return(bm);
        }
        FreeBitMap(bm);
    }
    return(NULL);
}

struct BitMap *loadBitMap(STRPTR name, struct ColorMap **cm)
{
    struct IFFHandle    *iff;
    struct BitMapHeader *bmhd;
    struct BitMap       *bm = NULL;

    if (iff = openBitMap(name, &bmhd))
    {
        if (readColorMap(iff, cm))
        {
            bm = readBitMap(iff, bmhd);
        }
        closeIFF(iff);
    }
    return(bm);
}
