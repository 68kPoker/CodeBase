
#include <exec/memory.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))
#define RowBytes(w) ((((w)+15)>>4)<<1)
#define MIN(a,b) ((a)<(b)?(a):(b))

#define BUF_SIZE 2048

struct buffer
{
    BYTE *beg, *cur;
    LONG size, left;
};

struct IFFHandle *openIFF(STRPTR name)
{
    struct IFFHandle *iff;

    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = (ULONG)Open(name, MODE_OLDFILE))
        {
            InitIFFasDOS(iff);
            if (OpenIFF(iff, IFFF_READ) == 0)
            {
                return(iff);
            }
            Close((BPTR)iff->iff_Stream);
        }
        FreeIFF(iff);
    }
    return(NULL);
}

void closeIFF(struct IFFHandle *iff)
{
    CloseIFF(iff);
    Close((BPTR)iff->iff_Stream);
    FreeIFF(iff);
}

struct IFFHandle *openILBM(STRPTR name)
{
    struct IFFHandle *iff;

    if (iff = openIFF(name))
    {
        if (ParseIFF(iff, IFFPARSE_STEP) == 0)
        {
            struct ContextNode *cn;
            if (cn = CurrentChunk(iff))
            {
                if (cn->cn_Type == ID_ILBM && cn->cn_ID == ID_FORM)
                {
                    if (PropChunk(iff, ID_ILBM, ID_BMHD) == 0)
                    {
                        if (PropChunk(iff, ID_ILBM, ID_CMAP) == 0)
                        {
                            if (StopChunk(iff, ID_ILBM, ID_BODY) == 0)
                            {
                                if (ParseIFF(iff, IFFPARSE_SCAN) == 0)
                                {
                                    return(iff);
                                }
                            }
                        }
                    }
                }
            }
        }
        closeIFF(iff);
    }
    return(NULL);
}

BOOL readPalette(struct IFFHandle *iff, struct ColorMap *cm)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        UBYTE *data = sp->sp_Data;
        LONG size   = sp->sp_Size;
        WORD colors = size / 3,
                      i;

        for (i = 0; i < colors; i++)
        {
            UBYTE red   = *data++,
                  green = *data++,
                  blue  = *data++;

            SetRGB32CM(cm, i, RGB(red), RGB(green), RGB(blue));
        }
        return(TRUE);
    }
    return(FALSE);
}

/* Buffered ReadChunkBytes() */
LONG readChunkBytes(struct IFFHandle *iff, struct buffer *buf, BYTE *dest, WORD count)
{
    LONG sum = 0, min;

    while (count > 0)
    {
        if (buf->left == 0)
        {
            if ((buf->left = ReadChunkBytes(iff, buf->beg, buf->size)) == 0)
                break;
            buf->cur = buf->beg;
        }
        min = MIN(count, buf->left);
        CopyMem(buf->cur, dest, min);
        buf->cur += min;
        dest += min;
        sum += min;
        count -= min;
        buf->left -= min;
    }
    return(sum);
}

WORD unpackRow(struct IFFHandle *iff, struct buffer *buf, PLANEPTR dest, WORD bpr, UBYTE cmp)
{
    if (cmp == cmpNone)
    {
        if (readChunkBytes(iff, buf, dest, bpr) != bpr)
            return(FALSE);
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE c;
            if (readChunkBytes(iff, buf, &c, 1) != 1)
                return(FALSE);
            if (c >= 0)
            {
                WORD count = c + 1;
                if (bpr < count || readChunkBytes(iff, buf, dest, count) != count)
                    return(FALSE);
                bpr -= count;
                dest += count;
            }
            else if (c != -128)
            {
                WORD count = (-c) + 1;
                BYTE data;
                if (bpr < count || readChunkBytes(iff, buf, &data, 1) != 1)
                    return(FALSE);
                bpr -= count;
                while (count-- > 0)
                    *dest++ = data;
            }
        }
    }
    return(TRUE);
}

BOOL readBitMap(struct IFFHandle *iff, struct BitMap *bm, PLANEPTR mask)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
    {
        struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;
        WORD w = GetBitMapAttr(bm, BMA_WIDTH),
             h = GetBitMapAttr(bm, BMA_HEIGHT),
             d = GetBitMapAttr(bm, BMA_DEPTH);

        if (w == bmhd->bmh_Width && h == bmhd->bmh_Height && d == bmhd->bmh_Depth)
        {
            UBYTE cmp, msk;
            if ((cmp == cmpNone || cmp == cmpByteRun1) && (msk == mskNone || msk == mskHasTransparentColor || msk == mskHasMask))
            {
                struct buffer buf;
                if (buf.beg = AllocMem(buf.size = BUF_SIZE, MEMF_PUBLIC))
                {
                    WORD p, r;
                    WORD bpr = RowBytes(w);
                    PLANEPTR planes[8];
                    BOOL success = FALSE;

                    buf.left = 0;

                    for (p = 0; p < d; p++)
                        planes[p] = bm->Planes[p];

                    for (r = 0; r < h; r++)
                    {
                        for (p = 0; p < d; p++)
                        {
                            if (!(success = unpackRow(iff, &buf, planes[p], bpr, cmp)))
                                break;
                            planes[p] += bm->BytesPerRow;
                        }
                        if (!success)
                            break;
                        if (msk == mskHasMask)
                        {
                            if (!(success = unpackRow(iff, &buf, mask, bpr, cmp)))
                                break;
                            mask += bpr;
                        }
                    }
                    FreeMem(buf.beg, buf.size);
                    if (success)
                        return(TRUE);
                }
            }
        }
    }
    return(FALSE);
}

struct BitMap *loadBitMap(STRPTR name, struct ColorMap *cm, PLANEPTR *mask)
{
    struct IFFHandle *iff;
    struct StoredProperty *sp;

    if (iff = openILBM(name))
    {
        if (readPalette(iff, cm))
        {
            if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
            {
                struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;
                struct BitMap *bm;
                if (bm = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, bmhd->bmh_Depth, BMF_INTERLEAVED, NULL))
                {
                    if (*mask = AllocRaster(bmhd->bmh_Width, bmhd->bmh_Height))
                    {
                        if (readBitMap(iff, bm, *mask))
                        {
                            closeIFF(iff);
                            return(bm);
                        }
                        FreeRaster(*mask, bmhd->bmh_Width, bmhd->bmh_Height);
                    }
                    FreeBitMap(bm);
                }
            }
        }
        closeIFF(iff);
    }
    return(NULL);
}
