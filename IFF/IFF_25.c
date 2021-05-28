
/* IFF.c: IFF loading/saving functions */

#include <dos/dos.h>
#include <intuition/screens.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <datatypes/soundclass.h>
#include <exec/memory.h>

#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>

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

BOOL scanILBM(struct IFFHandle *iff)
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

BOOL scan8SVX(struct IFFHandle *iff)
{
    if (!PropChunk(iff, ID_8SVX, ID_VHDR))
    {
        if (!StopChunk(iff, ID_8SVX, ID_BODY))
        {
            if (!ParseIFF(iff, IFFPARSE_SCAN))
            {
                return(TRUE);
            }
        }
    }
    return(FALSE);
}

BOOL loadCMAP(struct IFFHandle *iff, struct Screen *s)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        UBYTE *cmap = sp->sp_Data;
        WORD count = sp->sp_Size / 3;
        WORD i;
        struct ColorMap *cm = s->ViewPort.ColorMap;

        for (i = 0; i < count; i++)
        {
            UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;

            SetRGB32CM(cm, i, RGB(red), RGB(green), RGB(blue));
        }
        MakeScreen(s);
        RethinkDisplay();
        return(TRUE);
    }
    return(FALSE);
}

BYTE *loadBODY(struct IFFHandle *iff, LONG *size, LONG flags)
{
    struct ContextNode *cn;

    if (cn = CurrentChunk(iff))
    {
        BYTE *buffer;
        *size = cn->cn_Size;
        if (buffer = AllocMem(*size, flags))
        {
            if (ReadChunkBytes(iff, buffer, *size) == *size)
            {
                return(buffer);
            }
            FreeMem(buffer, *size);
        }
    }
    return(NULL);
}

BOOL unpackRow(BYTE **bufferptr, LONG *sizeptr, PLANEPTR plane, UBYTE cmp, WORD bpr)
{
    BYTE *buffer = *bufferptr;
    LONG size = *sizeptr;

    if (cmp == cmpNone)
    {
        if (size < bpr)
        {
            return(FALSE);
        }
        CopyMem(buffer, plane, bpr);
        size -= bpr;
        buffer += bpr;
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
                    *plane++ = *buffer++;
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
                    *plane++ = data;
                }
            }
        }
    }
    else
    {
        return(FALSE);
    }

    *bufferptr = buffer;
    *sizeptr = size;
    return(TRUE);
}

BOOL unpackBitMap(BYTE *buffer, LONG size, struct BitMapHeader *bmhd, struct BitMap *bm)
{
    PLANEPTR planes[9];
    WORD i, j;
    WORD bpr = RowBytes(bmhd->bmh_Width);

    for (i = 0; i < bmhd->bmh_Depth; i++)
    {
        planes[i] = bm->Planes[i];
    }

    for (j = 0; j < bmhd->bmh_Height; j++)
    {
        for (i = 0; i < bmhd->bmh_Depth; i++)
        {
            if (!unpackRow(&buffer, &size, planes[i], bmhd->bmh_Compression, bpr))
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
    struct StoredProperty *sp;
    BOOL result = FALSE;

    if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
    {
        struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;
        struct BitMap *bm;

        if (bm = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, BMF_INTERLEAVED, NULL))
        {
            BYTE *buffer;
            LONG size;
            if (buffer = loadBODY(iff, &size, MEMF_PUBLIC))
            {
                result = unpackBitMap(buffer, size, bmhd, bm);
                FreeMem(buffer, size);
                if (result)
                {
                    return(bm);
                }
            }
            FreeBitMap(bm);
        }
    }
    return(NULL);
}

BOOL loadSample(STRPTR name, struct soundSample *s)
{
    struct IFFHandle *iff;
    if (iff = openIFF(name, IFFF_READ))
    {
        if (scan8SVX(iff))
        {
            BYTE *buffer;
            LONG size;
            struct StoredProperty *sp;

            if (sp = FindProp(iff, ID_8SVX, ID_VHDR))
            {
                s->vhdr = *(struct VoiceHeader *)sp->sp_Data;
                if (buffer = loadBODY(iff, &size, MEMF_CHIP))
                {
                    s->data = buffer;
                    s->size = size;
                    closeIFF(iff);
                    return(TRUE);
                }
            }
        }
        closeIFF(iff);
    }
    return(FALSE);
}

void freeSample(struct soundSample *s)
{
    FreeMem(s->data, s->size);
}
