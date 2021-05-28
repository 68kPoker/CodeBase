
/* $Log:	IFF.c,v $
 * Revision 1.1  12/.0/.1  .2:.1:.3  Robert
 * Initial revision
 *  */

#include <exec/memory.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>

#include <clib/graphics_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/exec_protos.h>

#include "IFF.h"

ULONG ilbm_props[] =
{
    ID_ILBM,    ID_BMHD,
    ID_ILBM,    ID_CMAP,
    0
};

ULONG ilbm_stops[] =
{
    ID_ILBM,    ID_BODY,
    0
};

/*
** initIFF() - Init IFF stream.
*/
struct IFFHandle *initIFF(ULONG stream, WORD type)
{
    struct IFFHandle *iff;

    if (iff = AllocIFF())
    {
        iff->iff_Stream = stream;
        switch (type)
        {
            case STREAM_DOS:  InitIFFasDOS(iff);  break;
            case STREAM_CLIP: InitIFFasClip(iff); break;
        }
        return(iff);
    }
    return(NULL);
}

/*
** openIFF() - Open IFF file for read/write.
*/
struct IFFHandle *openIFF(ULONG stream, WORD type, LONG mode)
{
    struct IFFHandle *iff;

    if (iff = initIFF(stream, type))
    {
        /* Here: Init custom stream if type == CUSTOM */
        if (OpenIFF(iff, mode) == 0)
        {
            return(iff);
        }
        FreeIFF(iff);
    }
    return(NULL);
}

void closeIFF(struct IFFHandle *iff)
{
    CloseIFF(iff);
    FreeIFF(iff);
}

/*
** countIDs() - Count chunk identifier pairs.
*/
WORD countIDs(ULONG *idArray)
{
    WORD count = 0;
    if (!idArray)
    {
        return(0);
    }

    while (*idArray++)
    {
        count++;
    }
    return(count / 2);
}

/*
** scanIFF() - Perform simple scan.
*/
BOOL scanIFF(struct IFFHandle *iff, ULONG *props, ULONG *colls, ULONG *stops)
{
    if (PropChunks(iff, props, countIDs(props)) == 0)
    {
        if (CollectionChunks(iff, colls, countIDs(colls)) == 0)
        {
            if (StopChunks(iff, stops, countIDs(stops)) == 0)
            {
                if (ParseIFF(iff, IFFPARSE_SCAN) == 0)
                {
                    return(TRUE);
                }
            }
        }
    }
    return(FALSE);
}

/*
** findPropData() - Get chunk Data pointer.
*/
BYTE *findPropData(struct IFFHandle *iff, ULONG type, ULONG id)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, type, id))
    {
        return(sp->sp_Data);
    }
    return(NULL);
}

/*
** readBODY() - Read whole BODY chunk to memory.
*/
BYTE *readBODY(struct IFFHandle *iff, struct ContextNode **cn)
{
    if (*cn = CurrentChunk(iff))
    {
        BYTE *buffer;

        if (buffer = AllocMem((*cn)->cn_Size, MEMF_PUBLIC))
        {
            if (ReadChunkBytes(iff, buffer, (*cn)->cn_Size) == (*cn)->cn_Size)
            {
                return(buffer);
            }
            FreeMem(buffer, (*cn)->cn_Size);
        }
    }
    return(NULL);
}

/*
** openILBM() - Open and scan ILBM picture.
*/
struct IFFHandle *openILBM(struct BitMapHeader **bmhd, ULONG stream, WORD type)
{
    struct IFFHandle *iff;

    if (iff = openIFF(stream, type, IFFF_READ))
    {
        if (scanIFF(iff, ilbm_props, NULL, ilbm_stops))
        {
            if (*bmhd = findBMHD(iff))
            {
                return(iff);
            }
        }
        closeIFF(iff);
    }
    return(NULL);
}

/*
** readCMAP() - Read color palette.
*/
struct ColorMap *readCMAP(struct IFFHandle *iff, struct ColorMap *cm)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        LONG size = sp->sp_Size;
        WORD colors = size / 3;
        UBYTE *cmap = sp->sp_Data;

        if (!cm)
        {
            cm = GetColorMap(colors);
        }

        if (cm)
        {
            WORD i;
            for (i = 0; i < colors; i++)
            {
                UBYTE red = *cmap++;
                UBYTE green = *cmap++;
                UBYTE blue = *cmap++;

                SetRGB32CM(cm, i, RGB(red), RGB(green), RGB(blue));
            }
            return(cm);
        }
    }
    return(NULL);
}

BOOL unpackRow(BYTE **bufferptr, LONG *sizeptr, BYTE **planeptr, UBYTE cmp, WORD bpr)
{
    BYTE *buffer = *bufferptr;
    BYTE *plane = *planeptr;
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
    *planeptr = plane;
    *sizeptr = size;

    return(TRUE);
}

BOOL unpackBitMap(struct BitMapHeader *bmhd, BYTE *buffer, LONG size, struct BitMap *bm)
{
    WORD width = bmhd->bmh_Width;
    WORD height = bmhd->bmh_Height;
    UBYTE depth = bmhd->bmh_Depth;
    UBYTE cmp = bmhd->bmh_Compression;

    WORD bpr = RowBytes(width);

    PLANEPTR planes[9];
    WORD i, j;

    for (i = 0; i < depth; i++)
    {
        planes[i] = bm->Planes[i];
    }

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < depth; i++)
        {
            if (!unpackRow(&buffer, &size, &planes[i], cmp, bpr))
            {
                return(FALSE);
            }
        }
    }
    return(TRUE);
}

/*
** readBitMap: Read picture.
*/
struct BitMap *readBitMap(struct IFFHandle *iff)
{
    struct BitMapHeader *bmhd;

    if (bmhd = findBMHD(iff))
    {
        BYTE *buffer;
        struct ContextNode *cn;

        if (buffer = readBODY(iff, &cn))
        {
            struct BitMap *bm;
            WORD width = bmhd->bmh_Width;
            WORD height = bmhd->bmh_Height;
            UBYTE depth = bmhd->bmh_Depth;

            if (bm = AllocBitMap(width, height, depth, 0, NULL))
            {
                if (unpackBitMap(bmhd, buffer, cn->cn_Size, bm))
                {
                    FreeMem(buffer, cn->cn_Size);
                    return(bm);
                }
                FreeBitMap(bm);
            }
            FreeMem(buffer, cn->cn_Size);
        }
    }
    return(NULL);
}
