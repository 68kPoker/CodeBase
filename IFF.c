
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>

#define RGB(v) ((v)|((v)<<8)|((v)<<16)|((v)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)

/* Insall IFF chunks. */
BOOL installIFF(struct IFFHandle *iff, ULONG type, ULONG *props, WORD count)
{
    if (PropChunks(iff, props, count) == 0)
    {
        if (StopChunk(iff, type, ID_BODY) == 0)
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

/* Alloc, open and scan IFF file. */
struct IFFHandle *scanIFF(STRPTR name, ULONG type, ULONG *props, WORD count)
{
    struct IFFHandle *iff;

    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = Open(name, MODE_OLDFILE))
        {
            InitIFFasDOS(iff);
            if (OpenIFF(iff, IFFF_READ) == 0)
            {
                if (installIFF(iff, type, props, count))
                {
                    if (ParseIFF(iff, IFFPARSE_SCAN) == 0)
                    {
                        return(iff);
                    }
                }
                CloseIFF(iff);
            }
            Close(iff->iff_Stream);
        }
        FreeIFF(iff);
    }
    return(NULL);
}

/* Close and free IFF. */
VOID freeIFF(struct IFFHandle *iff)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}

/* Get and load ColorMap. */
BOOL loadColorMap(struct IFFHandle *iff, struct ColorMap **cm)
{
    struct StoredProperty *sp;
    WORD colors;
    UBYTE *cmap;
    WORD c;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        colors = sp->sp_Size / 3;
        cmap = sp->sp_Data;

        if (*cm = GetColorMap(colors))
        {
            for (c = 0; c < colors; c++)
            {
                UBYTE red = *cmap++;
                UBYTE green = *cmap++;
                UBYTE blue = *cmap++;
                SetRGB32CM(*cm, c, RGB(red), RGB(green), RGB(blue));
            }
            return(TRUE);
        }
    }
    return(FALSE);
}

BYTE *getBODYBuffer(struct IFFHandle *iff, struct ContextNode **cn)
{
    BYTE *buffer;

    if (*cn = CurrentChunk(iff))
    {
        if (buffer = AllocMem(cn->cn_Size, MEMF_PUBLIC))
        {
            if (ReadChunkBytes(iff, buffer, cn->cn_Size) == cn->cn_Size)
            {
                return(buffer);
            }
            FreeMem(buffer, cn->cn_Size);
        }
    }
    return(NULL);
}

BOOL unpackRow(BYTE **destptr, WORD bpr, BYTE **srcptr, LONG *sizeptr, UBYTE cmp, UBYTE msk)
{
    BYTE *dest = *destptr;
    BYTE *src = *srcptr;
    LONG size = *sizeptr;

    if (cmp == cmpNone)
    {
        if (size < bpr)
            return(FALSE);
        size -= bpr;
        CopyMem(src, dest, bpr);
        src  += bpr;
        dest += bpr;
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
                bpr  -= count;
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

    *destptr = dest;
    *srcptr = src;
    *sizeptr = size;
    return(TRUE);
}

BOOL unpackBitMap(struct BitMapHeader *bmhd, struct BitMap *bm, BYTE *buffer, LONG size)
{
    WORD width, height, depth;
    UBYTE cmp, msk;
    WORD row, plane;
    WORD bpr;
    PLANEPTR planes[8];

    width  = bmhd->bmh_Width;
    bpr    = RowBytes(width);
    height = bmhd->bmh_Height;
    depth  = bmhd->bmh_Depth;

    cmp = bmhd->bmh_Compression;
    msk = bmhd->bmh_Masking;

    if (cmp != cmpNone && cmp != cmpByteRun1)
        return(FALSE);

    if (msk != mskNone && msk != mskHasTransparentColor)
        return(FALSE);

    for (plane = 0; plane < depth; plane++)
        planes[plane] = bm->Planes[plane];

    for (row = 0; row < height; row++)
        for (plane = 0; plane < depth; plane++)
            if (!unpackRow(&planes[plane], bpr, &buffer, &size, cmp, msk))
                return(FALSE);

    return(TRUE);
}

/* Alloc and load BitMap. */
BOOL loadBitMap(struct IFFHandle *iff, struct BitMap **bm)
{
    struct StoredProperty *sp;
    struct BitMapHeader *bmhd;
    WORD width, height, depth;
    struct ContextNode *cn;
    BYTE *buffer;
    LONG size;

    if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
    {
        bmhd = (struct BitMapHeader *)sp->sp_Data;
        width  = bmhd->bmh_Width;
        height = bmhd->bmh_Height;
        depth  = bmhd->bmh_Depth;

        if (*bm = AllocBitMap(width, height, depth, 0, NULL))
        {
            /* Read packed ILBM BODY chunk */
            if (buffer = getBODYBuffer(iff, &cn))
            {
                size = cn->cn_Size;
                if (unpackBitMap(bmhd, *bm, buffer, size))
                {
                    FreeMem(buffer, size);
                    return(TRUE);
                }
                FreeMem(buffer, size);
            }
            FreeBitMap(*bm);
        }
    }
    return(FALSE);
}

/* Load IFF ILBM into BitMap/ColorMap. */
BOOL loadILBM(STRPTR name, struct BitMap **bm, struct ColorMap **cm)
{
    struct IFFHandle *iff;
    ULONG props[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP
    };
    BOOL result = FALSE;

    if (iff = scanIFF(name, ID_ILBM, props, 2))
    {
        loadColorMap(iff, cm);
        result = loadBitMap(iff, bm);
        freeIFF(iff);
    }
    return(result);
}

VOID unloadILBM(struct BitMap *bm, struct ColorMap *cm)
{
    FreeBitMap(bm);
    FreeColorMap(cm);
}

/* EOF */
