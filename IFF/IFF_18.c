
#include <stdio.h>
#include "debug.h"

#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <exec/memory.h>
#include <datatypes/pictureclass.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>

#include "IFF.h"

struct IFFHandle *openIFF(STRPTR name, BOOL write)
{
    struct IFFHandle *iff;
    LONG mode = IFFF_READ, dosmode = MODE_OLDFILE;

    if (write)
    {
        mode = IFFF_WRITE;
        dosmode = MODE_NEWFILE;
    }

    if (iff = AllocIFF())
    {
        if (!(iff->iff_Stream = Open(name, dosmode)))
        {
            printf("Couldn't open '%s' ", name);
            if (write)
                printf("for writing!\n");
            else
                printf("for reading!\n");
        }
        else
        {
            InitIFFasDOS(iff);
            if (OpenIFF(iff, mode) == 0)
            {
                return(iff);
            }
            Close(iff->iff_Stream);
        }
        FreeIFF(iff);
    }
    return(NULL);
}

VOID closeIFF(struct IFFHandle *iff)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}

WORD countChks(ULONG *chks)
{
    WORD cnt = 0;
    if (!chks)
    {
        return(0);
    }
    while (*chks++)
    {
        cnt++;
    }
    return(cnt / 2);
}

BOOL scanIFF(struct IFFHandle *iff, ULONG *props, ULONG *collects, ULONG *stops)
{
    if (PropChunks(iff, props, countChks(props)) == 0)
    {
        if (CollectionChunks(iff, collects, countChks(collects)) == 0)
        {
            if (StopChunks(iff, stops, countChks(stops)) == 0)
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

BYTE *findPropData(struct IFFHandle *iff, ULONG type, ULONG id)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, type, id))
    {
        return(sp->sp_Data);
    }
    return(NULL);
}

ULONG *getColors(struct IFFHandle *iff)
{
    struct StoredProperty *sp;
    ULONG *colors;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        LONG size = sp->sp_Size;
        if (colors = AllocVec((size + 2) * sizeof(ULONG), MEMF_PUBLIC))
        {
            WORD i;
            UBYTE *cmap = sp->sp_Data;
            colors[0] = (size / 3) << 16;

            for (i = 0; i < size; i++)
            {
                UBYTE data = *cmap++;
                colors[i + 1] = RGB(data);
            }
            colors[size + 1] = 0L;
            return(colors);
        }
    }
    return(NULL);
}

LONG readChunkBytes(struct IFFHandle *iff, struct Buffer *buf, BYTE *dest, LONG bytes)
{
    LONG actual = 0, count;

    while (bytes > 0)
    {
        if (buf->left == 0)
        {
            if ((buf->left = ReadChunkBytes(iff, buf->beg, buf->size)) == 0)
            {
                break;
            }
            buf->cur = buf->beg;
        }
        count = MIN(bytes, buf->left);
        CopyMem(buf->cur, dest, count);
        buf->cur    += count;
        dest        += count;
        actual      += count;
        buf->left   -= count;
        bytes       -= count;
    }
    return(actual);
}

BOOL unpackRow(struct IFFHandle *iff, struct Buffer *buf, BYTE *dest, WORD bpr, UBYTE cmp)
{
    if (cmp == cmpNone)
    {
        if (readChunkBytes(iff, buf, dest, bpr) != bpr)
        {
            return(FALSE);
        }
    }
    else if (cmp == cmpByteRun1)
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
    else
    {
        return(FALSE);
    }
    return(TRUE);
}

struct BitMap *unpackBitMap(struct IFFHandle *iff, struct BitMapHeader *bmhd)
{
    struct BitMap *bm;
    WORD width = bmhd->bmh_Width, height = bmhd->bmh_Height;
    UBYTE depth = bmhd->bmh_Depth;
    WORD i, j;
    UBYTE cmp = bmhd->bmh_Compression, msk = bmhd->bmh_Masking;
    struct Buffer buf;
    PLANEPTR planes[9];
    WORD bpr = RowBytes(width);

    if (msk == mskHasMask)
    {
        depth++;
    }

    if (bm = AllocBitMap(width, height, depth, 0, NULL))
    {
        for (i = 0; i < depth; i++)
        {
            planes[i] = bm->Planes[i];
        }

        if (buf.beg = AllocMem(buf.size = IFF_BUFFER_SIZE, MEMF_PUBLIC))
        {
            buf.left = 0;

            for (j = 0; j < height; j++)
            {
                for (i = 0; i < depth; i++)
                {
                    if (!unpackRow(iff, &buf, planes[i], bpr, cmp))
                    {
                        FreeMem(buf.beg, buf.size);
                        FreeBitMap(bm);
                        return(NULL);
                    }
                    planes[i] += bm->BytesPerRow;
                }
            }
            FreeMem(buf.beg, buf.size);
            return(bm);
        }
        FreeBitMap(bm);
    }
    return(NULL);
}

GFX *loadGraphics(STRPTR name)
{
    struct IFFHandle *iff;
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

    if (iff = openIFF(name, FALSE))
    {
        if (scanIFF(iff, props, NULL, stops))
        {
            struct BitMapHeader *bmhd;

            if (bmhd = (struct BitMapHeader *)findPropData(iff, ID_ILBM, ID_BMHD))
            {
                ULONG *colors;

                if (colors = getColors(iff))
                {
                    struct BitMap *bm;
                    if (bm = unpackBitMap(iff, bmhd))
                    {
                        GFX *gfx;

                        if (gfx = AllocMem(sizeof(*gfx), MEMF_PUBLIC))
                        {
                            gfx->bitmap = bm;
                            gfx->colorsRGB32 = colors;

                            closeIFF(iff);
                            return(gfx);
                        }
                        FreeBitMap(bm);
                    }
                    FreeVec(colors);
                }
            }
        }
        closeIFF(iff);
    }
    return(NULL);
}

VOID freeGraphics(GFX *gfx)
{
    FreeBitMap(gfx->bitmap);
    FreeVec(gfx->colorsRGB32);
    FreeMem(gfx, sizeof(*gfx));
}
