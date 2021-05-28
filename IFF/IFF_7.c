
/* IFF loading/saving */

#include "IFF.h"

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <exec/memory.h>
#include <datatypes/pictureclass.h>

#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

/* Open stream file */
ULONG openStream(struct IFFInfo *ii, STRPTR name, LONG mode)
{
    BOOL clip;
    UBYTE unit;
    ULONG stream;

    if (name[0] == '-' && name[1] == 'c')
    {
        clip = TRUE;
        unit = name[2] ? atoi(name + 2) : PRIMARY_CLIP;
        stream = (ULONG)OpenClipboard(unit);
    }
    else
    {
        clip = FALSE;
        stream = (ULONG)Open(name, mode == IFFF_WRITE ? MODE_NEWFILE : MODE_OLDFILE);
    }
    if (stream)
    {
        ii->clip = clip;
        return(stream);
    }
    return(0);
}

/* Open IFF file */
BOOL openIFF(struct IFFInfo *ii, STRPTR name, LONG mode)
{
    struct IFFHandle *iff;

    if (iff = AllocIFF())
    {
        ULONG stream;
        if (iff->iff_Stream = stream = openStream(ii, name, mode))
        {
            BOOL clip = ii->clip;
            LONG err;

            clip ? InitIFFasClip(iff) : InitIFFasDOS(iff);
            if ((err = OpenIFF(iff, mode)) == 0)
            {
                ii->iff = iff;
                return(TRUE);
            }
            else
                ii->err = err;
            clip ? CloseClipboard((struct ClipboardHandle *)stream) : Close((BPTR)stream);
        }
        FreeIFF(iff);
    }
    return(FALSE);
}

/* Close IFF file */
VOID closeIFF(struct IFFInfo *ii)
{
    struct IFFHandle *iff = ii->iff;
    ULONG stream = iff->iff_Stream;

    CloseIFF(iff);
    ii->clip ? CloseClipboard((struct ClipboardHandle *)stream) : Close((BPTR)stream);
    FreeIFF(iff);
}

LONG countChunks(ULONG *chunks)
{
    LONG n = 0;

    if (!chunks)
        return(0);

    while (*chunks++)
        n++;
    return(n >> 1);
}

/* Parse IFF file */
LONG parseIFF(struct IFFInfo *ii, ULONG type, ULONG id, ULONG *props, ULONG *colls, ULONG *stops)
{
    struct IFFHandle *iff = ii->iff;
    LONG err;

    if ((err = PropChunks(iff, props, countChunks(props))) == 0
     && (err = CollectionChunks(iff, colls, countChunks(colls))) == 0
     && (err = StopChunks(iff, stops, countChunks(stops))) == 0
     && (err = StopOnExit(iff, type, id)) == 0
     && ((err = ParseIFF(iff, IFFPARSE_SCAN)) == 0
        || err == IFFERR_EOC
        || err == IFFERR_EOF))
    {
        return(ii->err = err);
    }
    return(ii->err = err);
}

/* Parse FORM ILBM */
BOOL parseILBM(struct ILBMInfo *ilbm)
{
    ULONG props[] = { ID_ILBM, ID_BMHD, ID_ILBM, ID_CMAP, 0 };
    ULONG stops[] = { ID_ILBM, ID_BODY, 0 };
    LONG err;

    if ((err = parseIFF(&ilbm->ii, ID_ILBM, ID_FORM, props, NULL, stops)) == 0)
    {
        struct StoredProperty *sp;
        struct IFFHandle *iff = ilbm->ii.iff;

        if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
        {
            ilbm->bmhd = (struct BitMapHeader *)sp->sp_Data;
            return(TRUE);
        }
    }
    return(FALSE);
}

/* Load BODY */
BOOL loadBODY(struct IFFInfo *ii)
{
    struct IFFHandle *iff = ii->iff;
    struct ContextNode *cn;

    if (cn = CurrentChunk(iff))
    {
        BYTE *buffer;
        LONG size = cn->cn_Size;

        if (buffer = AllocMem(size, MEMF_PUBLIC))
        {
            if (ReadChunkBytes(iff, buffer, size) == size)
            {
                ii->buffer = buffer;
                ii->size = size;
                return(TRUE);
            }
            FreeMem(buffer, size);
        }
    }
    return(FALSE);
}

VOID freeBODY(struct IFFInfo *ii)
{
    FreeMem(ii->buffer, ii->size);
}

/* Load color palette */
BOOL loadColors(struct ILBMInfo *ilbm)
{
    struct StoredProperty *sp;
    struct IFFHandle *iff = ilbm->ii.iff;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        ULONG *colors;
        WORD size = sp->sp_Size;

        if (colors = AllocVec((size + 2) * sizeof(ULONG), MEMF_PUBLIC))
        {
            WORD col;
            ULONG *cur = colors;
            UBYTE *cmap = sp->sp_Data;

            *cur++ = (size / 3) << 16;
            for (col = 0; col < size; col++)
            {
                UBYTE data = *cmap++;
                *cur++ = RGB(data);
            }
            *cur = 0L;
            ilbm->colors = colors;
            return(TRUE);
        }
    }
    return(FALSE);
}

VOID freeColors(struct ILBMInfo *ilbm)
{
    FreeVec(ilbm->colors);
}

BOOL unpackRow(BYTE **srcptr, LONG *sizeptr, BYTE **destptr, WORD bpr, UBYTE cmp, UBYTE msk)
{
    BYTE *src = *srcptr;
    LONG size = *sizeptr;
    BYTE *dest = *destptr;

    if (cmp == cmpNone)
    {
        if (size < bpr)
            return(FALSE);
        CopyMem(src, dest, bpr);
        src += bpr;
        dest += bpr;
        size -= bpr;
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE con;
            if (size < 1)
                return(FALSE);
            size--;
            if ((con = *src++) >= 0)
            {
                WORD count = con + 1;
                if (size < count || bpr < count)
                    return(FALSE);
                size -= count;
                bpr -= count;
                while (count-- > 0)
                    *dest++ = *src++;
            }
            else if (con != -128)
            {
                WORD count = (-con) + 1;
                BYTE data;
                if (size < 1 || bpr < count)
                    return(FALSE);
                size--;
                bpr -= count;
                data = *src++;
                while (count-- > 0)
                    *dest++ = data;
            }
        }
    }
    else
        return(FALSE);

    *srcptr = src;
    *sizeptr = size;
    *destptr = dest;
    return(TRUE);
}

/* Load BitMap */
BOOL loadBitMap(struct ILBMInfo *ilbm)
{
    struct IFFInfo *ii = &ilbm->ii;
    struct IFFHandle *iff = ii->iff;
    struct BitMap *bm;
    struct BitMapHeader *bmhd = ilbm->bmhd;
    WORD width = bmhd->bmh_Width;
    WORD height = bmhd->bmh_Height;
    WORD depth = bmhd->bmh_Depth;
    UBYTE cmp = bmhd->bmh_Compression;
    UBYTE msk = bmhd->bmh_Masking;

    if (bm = AllocBitMap(width, height, depth, 0, NULL))
    {
        PLANEPTR planes[8];
        WORD p, r;
        WORD bpr = RowBytes(width);

        if (loadBODY(ii))
        {
            BYTE *buffer = ii->buffer;
            LONG size = ii->size;
            BOOL success = FALSE;

            for (p = 0; p < depth; p++)
                planes[p] = bm->Planes[p];

            for (r = 0; r < height; r++)
            {
                for (p = 0; p < depth; p++)
                    if (!(success = unpackRow(&buffer, &size, &planes[p], bpr, cmp, msk)))
                        break;
                if (!success)
                    break;
            }
            if (success)
            {
                freeBODY(ii);
                ilbm->bm = bm;
                return(TRUE);
            }
            freeBODY(ii);
        }
        FreeBitMap(bm);
    }
    return(FALSE);
}

BOOL loadILBM(struct ILBMInfo *ilbm, STRPTR name)
{
    struct IFFInfo *ii = &ilbm->ii;

    if (openIFF(ii, name, IFFF_READ))
    {
        if (parseILBM(ilbm))
        {
            if (loadColors(ilbm))
            {
                if (loadBitMap(ilbm))
                {
                    closeIFF(ii);
                    return(TRUE);
                }
                freeColors(ilbm);
            }
        }
        closeIFF(ii);
    }
    return(FALSE);
}

VOID unloadILBM(struct ILBMInfo *ilbm)
{
    FreeBitMap(ilbm->bm);
    freeColors(ilbm);
}
