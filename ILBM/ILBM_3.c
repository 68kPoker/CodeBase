
#include <datatypes/pictureclass.h>

#include <clib/graphics_protos.h>
#include <clib/iffparse_protos.h>

#include "IFF.h"
#include "ILBM.h"

ULONG ilbmProps[] =
{
    ID_ILBM, ID_BMHD,
    ID_ILBM, ID_CMAP,
    0
};

/* Wczytaj paletë kolorów */
struct ColorMap *getColorMap(struct IFFHandle *iff)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        UBYTE *cmap = sp->sp_Data;
        LONG size = sp->sp_Size;
        struct ColorMap *cm;
        WORD colors = size / 3;

        if (cm = GetColorMap(colors))
        {
            WORD i;
            for (i = 0; i < colors; i++)
            {
                UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;
                SetRGB32CM(cm, i, RGB(red), RGB(green), RGB(blue));
            }
            return(cm);
        }
    }
    return(NULL);
}

BOOL unpackRow(struct IFFHandle *iff, struct buffer *buf, BYTE *dest, UBYTE cmp, UBYTE bpr)
{
    if (cmp == cmpNone)
    {
        if (readChunkBytes(iff, buf, dest, bpr) != bpr)
        {
            return(FALSE);
        }
    }
    else
    {
        while (bpr > 0)
        {
            BYTE con;
            if (readChunkBytes(iff, buf, &con, 1) != 1)
            {
                return(FALSE);
            }
            if (con >= 0)
            {
                WORD count = con + 1;
                if (bpr < count || readChunkBytes(iff, buf, dest, count) != count)
                {
                    return(FALSE);
                }
                dest += count;
                bpr -= count;
            }
            else if (con != -128)
            {
                WORD count = (-con) + 1;
                BYTE data;
                if (bpr < count || readChunkBytes(iff, buf, &data, 1) != 1)
                {
                    return(FALSE);
                }
                bpr -= count;
                while (count-- > 0)
                {
                    *dest++ = data;
                }
            }
        }
    }
    return(TRUE);
}

/* Wczytaj ciaîo obrazka */
struct BitMap *getBitMap(struct IFFHandle *iff)
{
    struct StoredProperty *sp;
    struct BitMap *bm = NULL;

    if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
    {
        struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;
        WORD width = bmhd->bmh_Width;
        register WORD height = bmhd->bmh_Height, row;
        register UBYTE depth = bmhd->bmh_Depth, plane;
        UBYTE cmp = bmhd->bmh_Compression;
        UBYTE msk = bmhd->bmh_Masking;
        WORD bpr = RowBytes(width);

        if ((cmp == cmpNone || cmp == cmpByteRun1) &&
            (msk == mskNone || msk == mskHasTransparentColor || msk == mskHasMask))
        {
            struct buffer buf;

            if (initBuffer(&buf))
            {
                if (msk == mskHasMask)
                {
                    ++depth;
                }
                if (bm = AllocBitMap(width, height, depth, 0, NULL))
                {
                    PLANEPTR planes[9];
                    BOOL success = FALSE;

                    for (plane = 0; plane < depth; plane++)
                    {
                        planes[plane] = bm->Planes[plane];
                    }
                    for (row = 0; row < height; row++)
                    {
                        for (plane = 0; plane < depth; plane++)
                        {
                            if (!(success = unpackRow(iff, &buf, planes[plane], cmp, bpr)))
                            {
                                break;
                            }
                            planes[plane] += bpr;
                        }
                        if (!success)
                        {
                            break;
                        }
                    }
                    if (!success)
                    {
                        FreeBitMap(bm);
                        bm = NULL;
                    }
                }
                freeBuffer(&buf);
            }
        }
    }
    return(bm);
}

