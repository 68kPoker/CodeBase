
/* Magazyn */

/* $Id$ */

#include <stdio.h>

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <exec/memory.h>

#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include "IFF.h"

void closeIFF(struct IFFHandle *iff, struct IFFUD *iud)
{
    if (iff)
    {
        BPTR f;
        if (iud->opened)    CloseIFF(iff);
        if (f = (BPTR)iff->iff_Stream)
        {
            Close(f);
        }
        FreeIFF(iff);
    }
}

struct IFFHandle *openIFF(struct IFFUD *iud, STRPTR name, LONG mode)
{
    struct IFFHandle *iff = NULL;
    BPTR f;
    LONG dos[] = { MODE_OLDFILE, MODE_NEWFILE };

    if (!(iff = AllocIFF()))
    {
        printf("Out of memory!\n");
        return(NULL);
    }

    if (!(iff->iff_Stream = (ULONG)(f = Open(name, dos[mode & 1]))))
    {
        printf("Couldn't open %s!\n", name);
        closeIFF(iff, iud);
        return(NULL);
    }

    iud->clip = FALSE;
    InitIFFasDOS(iff);

    if (iud->err = OpenIFF(iff, mode))
    {
        printf("Couldn't open iff (%ld)!\n", iud->err);
        closeIFF(iff, iud);
        return(NULL);
    }

    iud->opened = TRUE;

    return(iff);
}

WORD countChunks(LONG *chunks)
{
    WORD n = 0;

    if (!chunks)    return(0);
    while (*chunks++)
    {
        n++;
    }
    return(n / 2);
}

BOOL parseIFF(struct IFFHandle *iff, struct IFFUD *iud)
{
    if (iud->err = PropChunks(iff, iud->props, countChunks(iud->props)))
    {
        printf("PropChunks failed (%ld)!\n", iud->err);
        return(FALSE);
    }

    if (iud->err = StopChunks(iff, iud->stops, countChunks(iud->stops)))
    {
        printf("StopChunks failed (%ld)!\n", iud->err);
        return(FALSE);
    }

    iud->err = ParseIFF(iff, IFFPARSE_SCAN);

    if (iud->err != 0 && iud->err != IFFERR_EOC && iud->err != IFFERR_EOF)
    {
        printf("ParseIFF failed (%ld)!\n", iud->err);
        return(FALSE);
    }

    return(TRUE);
}

BOOL initILBM(struct IFFHandle *iff, struct ILBMUD *bmud)
{
    struct StoredProperty *sp;

    if (!(sp = FindProp(iff, ID_ILBM, ID_BMHD)))
    {
        printf("BMHD not found!\n");
        return(FALSE);
    }

    bmud->bmhd = (struct BitMapHeader *)sp->sp_Data;
    return(TRUE);
}

BOOL loadCMAP(struct IFFHandle *iff, struct ILBMUD *bmud)
{
    struct StoredProperty *sp;
    UBYTE *cmap;
    WORD colors, color;
    struct ColorMap *cm;

    if (!(sp = FindProp(iff, ID_ILBM, ID_CMAP)))
    {
        printf("CMAP not found!\n");
        return(FALSE);
    }

    cmap = sp->sp_Data;
    colors = sp->sp_Size / 3;

    if (!(cm = bmud->cm))
    {
        if (!(bmud->cm = cm = GetColorMap(colors)))
        {
            printf("Out of memory!\n");
            return(FALSE);
        }
        bmud->allocated = TRUE;
    }

    for (color = 0; color < colors; color++)
    {
        UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;

        SetRGB32CM(cm, color, RGB(red), RGB(green), RGB(blue));
    }

    return(TRUE);
}

BOOL unpackRow(BYTE **bufptr, LONG *sizeptr, PLANEPTR current, WORD bpr)
{
    BYTE *buffer = *bufptr;
    LONG size = *sizeptr;

    while (bpr > 0)
    {
        BYTE con;
        if (size < 1)
        {
            return(FALSE);
        }
        size--;
        if ((con = *buffer++) >= 0)
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
                *current++ = *buffer++;
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
            data = *buffer++;
            while (count-- > 0)
            {
                *current++ = data;
            }
        }
    }

    *bufptr = buffer;
    *sizeptr = size;
    return(TRUE);
}

BOOL unpackBitMap(struct ILBMUD *bmud, BYTE *buffer, LONG size, WORD bpr, WORD height, UBYTE depth, UBYTE cmp)
{
    PLANEPTR planes[9], current;
    WORD row, plane;
    struct BitMap *bm = bmud->bm;

    for (plane = 0; plane < depth; plane++)
    {
        planes[plane] = bm->Planes[plane];
    }

    for (row = 0; row < height; row++)
    {
        for (plane = 0; plane < depth; plane++)
        {
            current = planes[plane];
            if (cmp == cmpNone)
            {
                if (size < bpr)
                {
                    return(FALSE);
                }
                CopyMem(buffer, current, bpr);
                buffer += bpr;
                size -= bpr;
            }
            else if (cmp == cmpByteRun1)
            {
                if (!unpackRow(&buffer, &size, current, bpr))
                {
                    return(FALSE);
                }
            }
            else
            {
                return(FALSE);
            }
            current += bm->BytesPerRow;
            planes[plane] = current;
        }
    }
    return(TRUE);
}

BOOL loadBitMap(struct IFFHandle *iff, struct ILBMUD *bmud)
{
    WORD width, height;
    UBYTE depth, cmp, msk;
    struct BitMapHeader *bmhd;
    struct BitMap *bm;
    struct ContextNode *cn;
    BYTE *buffer;
    LONG size;
    BOOL success = FALSE;
    const ULONG flags = 0;

    if (!initILBM(iff, bmud)) /* Get BitMapHeader */
    {
        return(FALSE);
    }

    bmhd = bmud->bmhd;

    width  = bmhd->bmh_Width;
    height = bmhd->bmh_Height;
    depth  = bmhd->bmh_Depth;
    cmp    = bmhd->bmh_Compression;
    msk    = bmhd->bmh_Masking;

    if (!(bmud->bm = bm = AllocBitMap(width, height, depth, flags, NULL)))
    {
        printf("Out of graphics memory!\n");
        return(FALSE);
    }

    if (!(cn = CurrentChunk(iff)))
    {
        printf("Couldn't get context node!\n");
        FreeBitMap(bm);
        return(FALSE);
    }

    size = cn->cn_Size;

    /* Temporary buffer for BODY */
    if (buffer = AllocMem(size, MEMF_PUBLIC))
    {
        if (ReadChunkBytes(iff, buffer, size) == size)
        {
            if (!(success = unpackBitMap(bmud, buffer, size, RowBytes(width), height, depth, cmp)))
            {
                printf("Decompression error!\n");
            }
        }
        else
        {
            printf("Chunk reading error!\n");
        }
        FreeMem(buffer, size);
    }
    else
    {
        printf("Out of memory!\n");
    }

    if (!success)
    {
        FreeBitMap(bm);
        return(FALSE);
    }

    return(TRUE);
}

/* Free ILBMUD components */
void unloadImage(struct ILBMUD *bmud)
{
    if (bmud->allocated)
    {
        FreeColorMap(bmud->cm);
    }

    if (bmud->bm)
    {
        FreeBitMap(bmud->bm);
    }
}

BOOL loadImage(struct ILBMUD *bmud, STRPTR name)
{
    struct IFFUD iud = { 0 };
    struct IFFHandle *iff;
    LONG props[] = { ID_ILBM, ID_BMHD, ID_ILBM, ID_CMAP, 0 };
    LONG stops[] = { ID_ILBM, ID_BODY, 0 };

    if (!(iff = openIFF(&iud, name, IFFF_READ)))
    {
        return(FALSE);
    }

    iud.props = props;
    iud.stops = stops;

    if (!parseIFF(iff, &iud) || !loadCMAP(iff, bmud) || !loadBitMap(iff, bmud))
    {
        closeIFF(iff, &iud);
        unloadImage(bmud);
        return(FALSE);
    }

    closeIFF(iff, &iud);

    return(TRUE);
}
