
/*
**  (C)2018-2020 Robert Szacki Software House
**
**  » Magazyn «
**
**  $Id: IFF.c,v 1.1 12/.0/.0 .2:.1:.2 Robert Exp $
*/

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <datatypes/soundclass.h>
#include <exec/memory.h>

#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#include "IFF.h"

struct IFFHandle* openIFF(STRPTR name, LONG mode)
{
    struct IFFHandle* iff;

    if (iff = AllocIFF())
    {
        BPTR f;

        if (iff->iff_Stream = f = Open(name, mode == IFFF_WRITE ? MODE_NEWFILE : MODE_OLDFILE))
        {
            LONG err;

            InitIFFasDOS(iff);
            if ((err = OpenIFF(iff, mode)) == 0)
            {
                return(iff);
            }
            Close(f);
        }
        FreeIFF(iff);
    }
    return(NULL);
}

void closeIFF(struct IFFHandle* iff)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}

LONG scanIFF(struct IFFHandle* iff, ULONG type, ULONG* props, WORD count)
{
    LONG err;
    if ((err = PropChunks(iff, props, count)) == 0)
    {
        if ((err = StopChunk(iff, type, ID_BODY)) == 0)
        {
            if ((err = StopOnExit(iff, type, ID_FORM)) == 0)
            {
                if ((err = ParseIFF(iff, IFFPARSE_SCAN)) == 0
                    || err == IFFERR_EOC
                    || err == IFFERR_EOF)
                {
                    return(err);
                }
            }
        }
    }
    return(err);
}

struct IFFHandle* openILBM(STRPTR name, struct BitMapHeader* bmhd)
{
    struct IFFHandle* iff;
    ULONG props[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP
    };

    if (iff = openIFF(name, IFFF_READ))
    {
        if (scanIFF(iff, ID_ILBM, props, 2) == 0)
        {
            struct StoredProperty* sp;

            if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
            {
                *bmhd = *(struct BitMapHeader* )sp->sp_Data;
                return(iff);
            }
        }
        closeIFF(iff);
    }
    return(NULL);
}

ULONG* loadColors(struct IFFHandle* iff, WORD* colorCount)
{
    struct StoredProperty* sp;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        WORD colors = sp->sp_Size / 3;
        ULONG* pal;

        if (pal = AllocMem((sp->sp_Size + 2) * sizeof(ULONG), MEMF_PUBLIC))
        {
            WORD i;
            UBYTE* cmap = sp->sp_Data;

            pal[0] = colors << 16;
            for (i = 1; i <= sp->sp_Size; i++)
            {
                UBYTE color = *cmap++;
                pal[i] = RGB(color);
            }
            pal[i] = 0L;
            *colorCount = colors;
            return(pal);
        }
    }
    return(NULL);
}

void freeColors(ULONG* pal, WORD colors)
{
    FreeMem(pal, ((colors * 3) + 2) * sizeof(ULONG));
}

BOOL unpackRow(BYTE** bufptr, LONG* sizeptr, BYTE** planeptr, WORD bpr, WORD cmp)
{
    BYTE *buf = *bufptr, *plane = *planeptr;
    LONG size = *sizeptr;

    if (cmp == cmpNone)
    {
        if (size < bpr)
        {
            return(FALSE);
        }
        size -= bpr;
        CopyMem(buf, plane, bpr);
        buf += bpr;
        plane += bpr;
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
            if ((con = *buf++) >= 0)
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
                    *plane++ = *buf++;
                }
            }
            else if (con != -128)
            {
                WORD count = (-con) + 1;
                BYTE data;
                if (size < 1 || bpr < count)
                {
                    return(FALSE);
                }
                size--;
                bpr -= count;
                data = *buf++;
                while (count-- > 0)
                {
                    *plane++ = data;
                }
            }
        }
    }
    else
    {
        return(FALSE);
    }

    *bufptr = buf;
    *sizeptr = size;
    *planeptr = plane;
    return(TRUE);
}

BOOL unpackBitMap(BYTE *buf, LONG size, struct BitMap* bm, struct BitMapHeader* bmhd)
{
    WORD width  = bmhd->bmh_Width,
         height = bmhd->bmh_Height,
         depth  = bmhd->bmh_Depth,
         bpr    = RowBytes(width),
         cmp    = bmhd->bmh_Compression,
         msk    = bmhd->bmh_Masking,
         plane, row;

    PLANEPTR planes[9];

    for (plane = 0; plane < depth; plane++)
    {
        planes[plane] = bm->Planes[plane];
    }

    for (row = 0; row < height; row++)
    {
        for (plane = 0; plane < depth; plane++)
        {
            if (!unpackRow(&buf, &size, &planes[plane], bpr, cmp))
            {
                return(FALSE);
            }
        }
    }
    return(TRUE);
}

struct BitMap* loadBitMap(struct IFFHandle* iff, struct BitMapHeader* bmhd)
{
    struct BitMap* bm;

    if (bm = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, 0, NULL))
    {
        struct ContextNode* cn;

        if ((cn = CurrentChunk(iff)) && cn->cn_Type == ID_ILBM && cn->cn_ID == ID_BODY)
        {
            BYTE* buf;
            LONG size = cn->cn_Size;
            BOOL success = FALSE;

            if (buf = AllocMem(size, MEMF_PUBLIC))
            {
                if (ReadChunkBytes(iff, buf, size) == size)
                {
                    success = unpackBitMap(buf, size, bm, bmhd);
                }
                FreeMem(buf, size);
                if (success)
                {
                    return(bm);
                }
            }
        }
        FreeBitMap(bm);
    }
    return(NULL);
}
