
/* IFF file reading */

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
        if (iff->iff_Stream = Open(name, MODE_OLDFILE))
        {
            InitIFFasDOS(iff);
            if (!OpenIFF(iff, IFFF_READ))
            {
                return(iff);
            }
            Close(iff->iff_Stream);
        }
        else
            printf("Couldn't open \"%s\"!\n", name);
        FreeIFF(iff);
    }
    return(NULL);
}

VOID closeIFF(struct IFFHandle *iff)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}

BOOL parseILBM(struct IFFHandle *iff)
{
    if (!PropChunk(iff, ID_ILBM, ID_BMHD))
    {
        if (!PropChunk(iff, ID_ILBM, ID_CMAP))
        {
            if (!StopChunk(iff, ID_ILBM, ID_BODY))
            {
                if (!ParseIFF(iff, IFFPARSE_SCAN))
                {
                    return(TRUE);
                }
            }
        }
    }
    return(FALSE);
}

struct BitMapHeader *findBMHD(struct IFFHandle *iff)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
    {
        return((struct BitMapHeader *)sp->sp_Data);
    }
    return(NULL);
}

BOOL loadColorMap(struct IFFHandle *iff, struct ColorMap *cm)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        UBYTE *cmap = sp->sp_Data;
        WORD i, colors = sp->sp_Size / 3;

        for (i = 0; i < colors; i++)
        {
            UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;
            SetRGB32CM(cm, i, RGB(red), RGB(green), RGB(blue));
        }
        return(TRUE);
    }
    return(FALSE);
}

BOOL unpackRow(BYTE **bufptr, LONG *sizeptr, BYTE *destptr, WORD bpr, UBYTE cmp, UBYTE msk)
{
    BYTE *buf = *bufptr;
    LONG size = *sizeptr;
    BYTE *dest = destptr;

    if (cmp == cmpNone)
    {
        if (size < bpr)
        {
            return(FALSE);
        }
        size -= bpr;
        CopyMem(buf, dest, bpr);
        buf += bpr;
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
            if ((con = *buf++) >= 0)
            {
                WORD len = con + 1;
                if (size < len || bpr < len)
                {
                    return(FALSE);
                }
                size -= len;
                bpr -= len;
                while (len-- > 0)
                {
                    *dest++ = *buf++;
                }
            }
            else if (con != -128)
            {
                WORD len = (-con) + 1;
                BYTE data;
                if (size < 1 || bpr < len)
                {
                    return(FALSE);
                }
                size--;
                bpr -= len;
                data = *buf++;
                while (len-- > 0)
                {
                    *dest++ = data;
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
    return(TRUE);
}

BOOL unpackBitMap(BYTE *buffer, LONG size, struct BitMap *bm, struct BitMapHeader *bmhd)
{
    WORD width = bmhd->bmh_Width, height = bmhd->bmh_Height, depth = bmhd->bmh_Depth;
    UBYTE cmp = bmhd->bmh_Compression, msk = bmhd->bmh_Masking;

    PLANEPTR planes[8];
    WORD i, j;
    WORD bpr = RowBytes(width);

    for (i = 0; i < depth; i++)
    {
        planes[i] = bm->Planes[i];
    }

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < depth; i++)
        {
            if (!unpackRow(&buffer, &size, planes[i], bpr, cmp, msk))
            {
                return(FALSE);
            }
            planes[i] += bm->BytesPerRow;
        }
    }
    return(TRUE);
}

struct BitMap *loadBitMap(struct IFFHandle *iff)
{
    struct BitMapHeader *bmhd;

    if (bmhd = findBMHD(iff))
    {
        BYTE *buffer;
        LONG size;
        struct ContextNode *cn;

        if (cn = CurrentChunk(iff))
        {
            size = cn->cn_Size;
            if (buffer = AllocMem(size, MEMF_PUBLIC))
            {
                if (ReadChunkBytes(iff, buffer, size) == size)
                {
                    struct BitMap *bm;

                    if (bm = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, BMF_INTERLEAVED|BMF_CLEAR, NULL))
                    {
                        if (unpackBitMap(buffer, size, bm, bmhd))
                        {
                            FreeMem(buffer, size);
                            return(bm);
                        }
                        FreeBitMap(bm);
                    }
                }
                FreeMem(buffer, size);
            }
        }
    }
    return(NULL);
}

struct BitMap *loadPicture(STRPTR name, struct ColorMap *cm)
{
    struct IFFHandle *iff;

    if (iff = openIFF(name))
    {
        if (parseILBM(iff))
        {
            if (loadColorMap(iff, cm))
            {
                struct BitMap *bm;
                if (bm = loadBitMap(iff))
                {
                    closeIFF(iff);
                    return(bm);
                }
            }
        }
        closeIFF(iff);
    }
    return(NULL);
}
