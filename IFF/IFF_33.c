
#include <stdio.h>
#include "debug.h"

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <exec/memory.h>

#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>

#include "IFF.h"

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

struct IFFHandle *openIFF(STRPTR name, LONG mode)
{
    struct IFFHandle *iff;
    LONG dosModes[] = { MODE_OLDFILE, MODE_NEWFILE };

    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = Open(name, dosModes[mode & 1]))
        {
            InitIFFasDOS(iff);
            if (!OpenIFF(iff, mode))
            {
                D(bug("%s IFF file opened.\n", name));
                return iff;
            }
            Close(iff->iff_Stream);
        }
        FreeIFF(iff);
    }
    return NULL;
}

void closeIFF(struct IFFHandle *iff)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}

BOOL scanILBM(struct IFFHandle *iff)
{
    if (!PropChunk(iff, ID_ILBM, ID_BMHD)
        && !PropChunk(iff, ID_ILBM, ID_CMAP)
        && !StopChunk(iff, ID_ILBM, ID_BODY))
    {
        if (!ParseIFF(iff, IFFPARSE_SCAN))
        {
            return TRUE;
        }
    }
    return FALSE;
}

UBYTE *findProp(struct IFFHandle *iff, ULONG type, ULONG id)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, type, id))
    {
        return sp->sp_Data;
    }
    return NULL;
}

struct ColorMap *loadCMap(struct IFFHandle *iff)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        struct ColorMap *cm;
        WORD colors = sp->sp_Size / 3;

        if (cm = GetColorMap(colors))
        {
            UBYTE *cmap = sp->sp_Data;
            WORD i;

            for (i = 0; i < colors; i++)
            {
                UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;
                SetRGB32CM(cm, i, RGB(red), RGB(green), RGB(blue));
            }
            return cm;
        }
    }
    return NULL;
}

BYTE *readChunk(struct IFFHandle *iff, LONG *sizeptr)
{
    struct ContextNode *cn;

    if (cn = CurrentChunk(iff))
    {
        BYTE *buf;
        LONG size = cn->cn_Size;

        if (buf = AllocMem(size, MEMF_PUBLIC))
        {
            if (ReadChunkBytes(iff, buf, size) == size)
            {
                *sizeptr = size;
                return buf;
            }
            FreeMem(buf, size);
        }
    }
    return NULL;
}

BOOL unpackBMap(BYTE *buf, LONG size, struct BitMap *bm, WORD w, WORD h, WORD d, UBYTE cmp, UBYTE msk)
{
    WORD i, j;
    PLANEPTR planes[9];
    WORD bpr;

    for (i = 0; i < d; i++)
    {
        planes[i] = bm->Planes[i];
    }

    for (j = 0; j < h; j++)
    {
        for (i = 0; i < d; i++)
        {
            bpr = RowBytes(w);
            if (cmp == cmpNone)
            {
                if (size < bpr)
                    return FALSE;

                CopyMem(buf, planes[i], bpr);
                buf += bpr;
                size -= bpr;
            }
            else if (cmp == cmpByteRun1)
            {
                PLANEPTR plane = planes[i];
                while (bpr > 0)
                {
                    BYTE con;
                    if (size < 1)
                        return FALSE;

                    size--;
                    if ((con = *buf++) >= 0)
                    {
                        WORD count = con + 1;
                        if (size < count || bpr < count)
                            return FALSE;
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
                            return FALSE;
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
            planes[i] += bm->BytesPerRow;
        }
    }
    return TRUE;
}

struct BitMap *loadBMap(struct IFFHandle *iff)
{
    struct BitMapHeader *bmhd;
    BOOL success = FALSE;

    if (bmhd = (struct BitMapHeader *)findProp(iff, ID_ILBM, ID_BMHD))
    {
        UWORD w = bmhd->bmh_Width, h = bmhd->bmh_Height, d = bmhd->bmh_Depth,
            cmp = bmhd->bmh_Compression, msk = bmhd->bmh_Masking;

        struct BitMap *bm;

        if (bm = AllocBitMap(w, h, d, 0, NULL))
        {
            BYTE *buf;
            LONG size;

            if (buf = readChunk(iff, &size))
            {
                success = unpackBMap(buf, size, bm, w, h, d, cmp, msk);

                FreeMem(buf, size);

                if (success)
                {
                    return bm;
                }
            }
            FreeBitMap(bm);
        }
    }
    return NULL;
}

struct BitMap *loadBitMap(STRPTR name, struct ColorMap **cm)
{
    struct IFFHandle *iff;

    if (iff = openIFF(name, IFFF_READ))
    {
        if (scanILBM(iff))
        {
            if (*cm = loadCMap(iff))
            {
                struct BitMap *bm;

                if (bm = loadBMap(iff))
                {
                    closeIFF(iff);
                    return bm;
                }
                FreeColorMap(*cm);
            }
        }
        closeIFF(iff);
    }
    return NULL;
}
