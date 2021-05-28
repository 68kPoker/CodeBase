
#include <libraries/iffparse.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <datatypes/pictureclass.h>

#include <clib/iffparse_protos.h>
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>

#define RowBytes(w) ((((w)+15)>>4)<<1)
#define RGB(c)      ((c)|((c)<<8)|((c)<<16)|((c)<<24))

struct buffer
{
    struct IFFHandle *iff;
    BYTE *beg, *cur;
    LONG size, left;
};

/* Buffered IFF chunk reading */
LONG readChunkBytes(struct buffer *buffer, BYTE *dest, LONG bytes)
{
    LONG read = 0; /* Total bytes read */

    while (bytes > 0)
    {
        LONG min; /* Bytes to read */

        if (buffer->left == 0)
        {
            if ((buffer->left = ReadChunkBytes(buffer->iff, buffer->beg, buffer->size)) == 0)
            {
                break;
            }
            buffer->cur = buffer->beg;
        }
        if (bytes < buffer->left)
        {
            min = bytes;
        }
        else
        {
            min = buffer->left;
        }
        CopyMem(buffer->cur, dest, min);
        buffer->cur += min;
        dest += min;
        read += min;
        bytes -= min;
        buffer->left -= min;
    }

    return(read);
}

/* Unpack one scan-line of the image */
BOOL unpackRow(struct buffer *buffer, BYTE *dest, WORD bpr, UBYTE cmp)
{
    if (cmp == cmpNone)
    {
        if (readChunkBytes(buffer, dest, bpr) != bpr)
        {
            return(FALSE);
        }
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE control;
            if (readChunkBytes(buffer, &control, 1) != 1)
            {
                return(FALSE);
            }
            if (control >= 0)
            {
                WORD count = control + 1;
                if (bpr < count || readChunkBytes(buffer, dest, count) != count)
                {
                    return(FALSE);
                }
                dest += count;
                bpr -= count;
            }
            else if (control != -128)
            {
                WORD count = (-control) + 1;
                BYTE repeat;
                if (bpr < count || readChunkBytes(buffer, &repeat, 1) != 1)
                {
                    return(FALSE);
                }
                bpr -= count;
                while (count-- > 0)
                {
                    *dest++ = repeat;
                }
            }
        }
    }
    else
    {
        return(FALSE);
    }
    return(TRUE);
}

/* Unpack picture to BitMap */
BOOL unpackBitMap(struct buffer *buffer, struct BitMap *bm, WORD bpr, UBYTE cmp)
{
    UWORD height = GetBitMapAttr(bm, BMA_HEIGHT);
    UBYTE depth  = GetBitMapAttr(bm, BMA_DEPTH);
    UWORD i, j;
    PLANEPTR planes[9];

    for (i = 0; i < depth; i++)
    {
        planes[i] = bm->Planes[i];
    }

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < depth; i++)
        {
            if (!unpackRow(buffer, planes[i], bpr, cmp))
            {
                return(FALSE);
            }
            planes[i] += bm->BytesPerRow;
        }
    }
    return(TRUE);
}

/* Alloc BitMap, buffer and unpack */
struct BitMap *readBitMap(struct IFFHandle *iff, struct BitMapHeader *bmhd, LONG flags)
{
    struct buffer buffer;
    const LONG bufferSize = 2048;
    BOOL result = FALSE;
    struct BitMap *bm = NULL;

    buffer.iff = iff;
    if (buffer.beg = AllocMem(buffer.size = bufferSize, MEMF_PUBLIC))
    {
        WORD depth = bmhd->bmh_Depth;

        if (bmhd->bmh_Masking == mskHasMask)
        {
            depth++;
        }

        buffer.left = 0;
        if (bm = AllocBitMap(bmhd->bmh_Width, bmhd->bmh_Height, depth, flags, NULL))
        {
            result = unpackBitMap(&buffer, bm, RowBytes(bmhd->bmh_Width), bmhd->bmh_Compression);
            if (!result)
            {
                FreeBitMap(bm);
                bm = NULL;
            }
        }
        FreeMem(buffer.beg, buffer.size);
    }
    return(bm);
}

/* Read color palette */
struct ColorMap *readPalette(struct IFFHandle *iff)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        UBYTE *cmap = sp->sp_Data;
        LONG size = sp->sp_Size;
        WORD colorCount = size / 3;
        struct ColorMap *cm;

        if (cm = GetColorMap(colorCount))
        {
            WORD i;

            for (i = 0; i < colorCount; i++)
            {
                UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;
                SetRGB32CM(cm, i, RGB(red), RGB(green), RGB(blue));
            }
            return(cm);
        }
    }
    return(NULL);
}

/* Open IFF, scan, load palette and bitmap */
struct BitMap *loadBitMap(STRPTR name, struct ColorMap **cm, LONG flags, BOOL *mask)
{
    struct IFFHandle *iff;
    struct BitMap *bm = NULL;
    ULONG props[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP
    };
    const WORD propCount = 2;

    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = Open(name, MODE_OLDFILE))
        {
            InitIFFasDOS(iff);
            if (OpenIFF(iff, IFFF_READ) == 0)
            {
                if (PropChunks(iff, props, propCount) == 0)
                {
                    if (StopChunk(iff, ID_ILBM, ID_BODY) == 0)
                    {
                        if (ParseIFF(iff, IFFPARSE_SCAN) == 0)
                        {
                            struct StoredProperty *sp;

                            if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
                            {
                                struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;

                                *mask = (bmhd->bmh_Masking == mskHasMask);
                                if (*cm = readPalette(iff))
                                {
                                    bm = readBitMap(iff, bmhd, flags);
                                    if (!bm)
                                    {
                                        FreeColorMap(*cm);
                                    }
                                }
                            }
                        }
                    }
                }
                CloseIFF(iff);
            }
            Close(iff->iff_Stream);
        }
        FreeIFF(iff);
    }
    return(bm);
}

void unloadBitMap(struct BitMap *bm, struct ColorMap *cm)
{
    FreeColorMap(cm);
    FreeBitMap(bm);
}
