
#include <datatypes/pictureclass.h>
#include <clib/graphics_protos.h>
#include <clib/iffparse_protos.h>

#include "IFF.h"
#include "ILBM.h"
#include <stdio.h>
#include "debug.h"

BOOL queryILBM(ILBM *ilbm, STRPTR name)
{
    IFF *iff = &ilbm->iff;
    struct ContextNode *cn;
    struct BitMapHeader *bmhd;

    if (prepIFF(iff))
    {
        if (openIFF(iff, name))
        {
            if (cn = enterIFF(iff))
            {
                if (cn->cn_Type == ID_ILBM && cn->cn_ID == ID_FORM)
                {
                    if (ilbm->bmhd = bmhd = scanILBM(iff))
                    {
                        return(TRUE);
                    }
                }
            }
            closeIFF(iff);
        }
        freeIFF(iff);
    }
    return(FALSE);
}

BOOL unpackRow(IFF *iff, BYTE **destptr, WORD bpr, UBYTE cmp)
{
    BYTE *dest = *destptr;

    if (cmp == cmpNone)
    {
        if (!readIFF(iff, dest, bpr))
        {
            return(FALSE);
        }
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE con;
            if (!readIFF(iff, &con, 1))
            {
                return(FALSE);
            }
            if (con >= 0)
            {
                WORD count = con + 1;
                if (bpr < count)
                {
                    return(FALSE);
                }
                bpr -= count;
                if (!readIFF(iff, dest, count))
                {
                    return(FALSE);
                }
                dest += count;
            }
            else if (con != -128)
            {
                BYTE data;
                WORD count = (-con) + 1;
                if (bpr < count)
                {
                    return(FALSE);
                }
                bpr -= count;
                if (!readIFF(iff, &data, 1))
                {
                    return(FALSE);
                }
                while (count-- > 0)
                {
                    *dest++ = data;
                }
            }
        }
        if (bpr != 0)
        {
            return(FALSE);
        }
    }

    *destptr = dest;
    return(TRUE);
}

BOOL loadCMAP(ILBM *ilbm)
{
    IFF *iff = &ilbm->iff;
    struct StoredProperty *sp;

    if (sp = FindProp(iff->iff, ID_ILBM, ID_CMAP))
    {
        UBYTE *cmap = sp->sp_Data;
        LONG size = sp->sp_Size;
        WORD colors = size / 3;
        if (ilbm->cm = GetColorMap(colors))
        {
            WORD i;
            for (i = 0; i < colors; i++)
            {
                UBYTE red = *cmap++;
                UBYTE green = *cmap++;
                UBYTE blue = *cmap++;
                SetRGB32CM(ilbm->cm, i, RGB(red), RGB(green), RGB(blue));
            }
            return(TRUE);
        }
    }
    return(FALSE);
}

BOOL loadILBM(ILBM *ilbm)
{
    IFF *iff = &ilbm->iff;
    struct BitMapHeader *bmhd = ilbm->bmhd;
    struct BitMap *bm;
    WORD width = bmhd->bmh_Width;
    WORD height = bmhd->bmh_Height;
    WORD depth = bmhd->bmh_Depth;
    UBYTE cmp = bmhd->bmh_Compression;
    UBYTE msk = bmhd->bmh_Masking;
    WORD bpr = RowBytes(width);
    PLANEPTR planes[9];
    WORD i, j;
    BOOL success = FALSE;

    if (msk == mskHasMask)
    {
        depth++;
        ilbm->mask = TRUE;
    }

    if (ilbm->bm = bm = AllocBitMap(width, height, depth, 0, NULL))
    {
        for (i = 0; i < depth; i++)
        {
            planes[i] = bm->Planes[i];
        }

        for (j = 0; j < height; j++)
        {
            for (i = 0; i < depth; i++)
            {
                if (!(success = unpackRow(iff, &planes[i], bpr, cmp)))
                {
                    break;
                }
            }
            if (!success)
            {
                break;
            }
        }
        if (success)
        {
            return(TRUE);
        }
        FreeBitMap(bm);
    }
    return(FALSE);
}

VOID freeILBM(ILBM *ilbm)
{
    if (ilbm->bm)
    {
        FreeBitMap(ilbm->bm);
    }
    if (ilbm->cm)
    {
        FreeColorMap(ilbm->cm);
    }
}
