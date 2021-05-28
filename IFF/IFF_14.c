
#include "iff.h"

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <exec/memory.h>
#include <datatypes/pictureclass.h>

#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#define RowBytes(w) ((((w)+15)>>4)<<1)
#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

#define BODYSIZE 4096

struct tempIFF
    {
    struct IFFHandle *iff;
    ULONG *props, *stops;
    BYTE *buffer, *current;
    LONG size, left;
    };

VOID closeIFF(struct tempIFF *temp);

BOOL openIFF(struct tempIFF *temp, STRPTR name, LONG mode)
{
    LONG dosmode = (mode == IFFF_WRITE ? MODE_NEWFILE : MODE_OLDFILE);
    LONG err;

    if (temp->iff = AllocIFF())
        {
        if (temp->iff->iff_Stream = Open(name, dosmode))
            {
            InitIFFasDOS(temp->iff);
            if ((err = OpenIFF(temp->iff, mode)) == 0)
                {
                return(TRUE);
                }
            Close(temp->iff->iff_Stream);
            }
        FreeIFF(temp->iff);
        }
    return(FALSE);
}

LONG countChunks(ULONG *chunks)
{
    LONG count = 0;

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

BOOL installIFF(struct tempIFF *temp)
{
    LONG err;
    if ((err = PropChunks(temp->iff, temp->props, countChunks(temp->props))) == 0)
        {
        if ((err = StopChunks(temp->iff, temp->stops, countChunks(temp->stops))) == 0)
            {
            if ((err = ParseIFF(temp->iff, IFFPARSE_SCAN)) == 0)
                {
                return(TRUE);
                }
            }
        }
    return(FALSE);
}

BOOL allocBODY(struct tempIFF *temp, LONG size)
{
    if (temp->buffer = AllocMem(size, MEMF_PUBLIC))
        {
        temp->size = size;
        temp->current = temp->buffer;
        temp->left = 0;
        return(TRUE);
        }
    return(FALSE);
}

VOID freeBODY(struct tempIFF *temp)
{
    FreeMem(temp->buffer, temp->size);
}

BOOL readCMAP(struct ILBMInfo *ii, struct StoredProperty *sp)
{
    WORD colors = sp->sp_Size / 3, color;
    if (ii->cm = GetColorMap(colors))
        {
        UBYTE *cmap = sp->sp_Data;
        for (color = 0; color < colors; color++)
            {
            UBYTE red = *cmap++;
            UBYTE green = *cmap++;
            UBYTE blue = *cmap++;
            SetRGB32CM(ii->cm, color, RGB(red), RGB(green), RGB(blue));
            }
        return(TRUE);
        }
    return(FALSE);
}

struct BitMapHeader *parseILBM(struct ILBMInfo *ii, struct tempIFF *temp, STRPTR name)
{
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

    if (openIFF(temp, name, IFFF_READ))
        {
        temp->props = props;
        temp->stops = stops;
        if (installIFF(temp))
            {
            struct StoredProperty *sp;

            if (sp = FindProp(temp->iff, ID_ILBM, ID_BMHD))
                {
                struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;

                if (sp = FindProp(temp->iff, ID_ILBM, ID_CMAP))
                    {
                    readCMAP(ii, sp);
                    return(bmhd);
                    }
                }
            }
        closeIFF(temp);
        }
    return(NULL);
}

LONG readChunkBytes(struct tempIFF *temp, BYTE *buffer, LONG size)
{
    LONG tocopy, copied = 0;

    while (size > 0)
        {
        if (temp->left == 0)
            {
            if ((temp->left = ReadChunkBytes(temp->iff, temp->buffer, temp->size)) == 0)
                {
                return(0);
                }
            temp->current = temp->buffer;
            }
        tocopy = (size < temp->left) ? size : temp->left;
        CopyMem(temp->current, buffer, tocopy);
        temp->current += tocopy;
        buffer += tocopy;
        temp->left -= tocopy;
        size -= tocopy;
        copied += tocopy;
        }
    return(copied);
}

BOOL unpackRow(struct tempIFF *temp, BYTE **planeptr, WORD bpr, UBYTE cmp)
{
    BYTE *plane = *planeptr;

    if (cmp == cmpNone)
        {
        if (!(readChunkBytes(temp, plane, bpr)))
            {
            return(FALSE);
            }
        plane += bpr;
        }
    else if (cmp == cmpByteRun1)
        {
        while (bpr > 0)
            {
            BYTE control;
            if (readChunkBytes(temp, &control, 1) != 1)
                {
                return(FALSE);
                }
            if (control >= 0)
                {
                WORD count = control + 1;
                if (bpr < count || readChunkBytes(temp, plane, count) != count)
                    {
                    return(FALSE);
                    }
                plane += count;
                bpr -= count;
                }
            else if (control != -128)
                {
                WORD count = (-control) + 1;
                BYTE data;
                if (bpr < count || readChunkBytes(temp, &data, 1) != 1)
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
        }
    else
        {
        return(FALSE);
        }

    *planeptr = plane;
    return(TRUE);
}

BOOL readILBM(struct ILBMInfo *ii, struct BitMapHeader *bmhd, struct tempIFF *temp)
{
    WORD width = bmhd->bmh_Width;
    WORD height = bmhd->bmh_Height, row;
    UBYTE depth = bmhd->bmh_Depth, plane;
    UBYTE cmp = bmhd->bmh_Compression;
    UBYTE msk = bmhd->bmh_Masking;
    WORD bpr = RowBytes(width);
    PLANEPTR planes[9];
    BOOL success = FALSE;

    if (msk == mskHasMask)
        {
        depth++;
        }

    if (ii->bm = AllocBitMap(width, height, depth, 0, NULL))
        {
        if (allocBODY(temp, BODYSIZE))
            {
            for (plane = 0; plane < depth; plane++)
                {
                planes[plane] = ii->bm->Planes[plane];
                }

            for (row = 0; row < height; row++)
                {
                for (plane = 0; plane < depth; plane++)
                    {
                    if (!(success = unpackRow(temp, &planes[plane], bpr, cmp)))
                        {
                        break;
                        }
                    }
                if (!success)
                    {
                    break;
                    }
                }
            freeBODY(temp);
            if (success)
                {
                return(TRUE);
                }
            }
        FreeBitMap(ii->bm);
        }
    return(FALSE);
}

VOID closeIFF(struct tempIFF *temp)
{
    CloseIFF(temp->iff);
    Close(temp->iff->iff_Stream);
    FreeIFF(temp->iff);
}

BOOL loadILBM(struct ILBMInfo *ii, STRPTR name)
{
    struct tempIFF temp;
    struct BitMapHeader *bmhd;
    BOOL result;

    if (bmhd = parseILBM(ii, &temp, name))
        {
        result = readILBM(ii, bmhd, &temp);
        closeIFF(&temp);
        return(result);
        }
    return(FALSE);
}

VOID freeILBM(struct ILBMInfo *ii)
{
    FreeColorMap(ii->cm);
    FreeBitMap(ii->bm);
}
