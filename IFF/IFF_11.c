
/* $Id$ */

#include <exec/memory.h>
#include <libraries/iffparse.h>
#include <datatypes/pictureclass.h>
#include <datatypes/soundclass.h>

#include <clib/exec_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>

#include "IFF.h"

#define MIN(a,b) ((a)<(b)?(a):(b))

#define RowBytes(w) ((((w)+15)>>4)<<1)

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

#define BODYBUF_SIZE 4096

typedef struct BitMapHeader *BMHD;

struct bodyBuffer
{
    struct IFFHandle *iff;
    BYTE *buffer, *current;
    LONG left;
};

/*
 * Open and scan IFF file of given type.
 */

IFFERR scanIFF(struct IFFHandle *iff, ULONG type)
{
    ULONG ilbmprops[] =
    {
        ID_ILBM, ID_BMHD,
        ID_ILBM, ID_CMAP,
        0
    };
    ULONG svxprops[] =
    {
        ID_8SVX, ID_VHDR,
        0
    };
    ULONG *props[] =
    {
        ilbmprops,
        svxprops
    };
    ULONG types[] =
    {
        ID_ILBM,
        ID_8SVX
    };

    LONG err;
    if ((err = OpenIFF(iff, IFFF_READ)) == 0)
    {
        if ((err = PropChunks(iff, props[type], countChunks(props[type]))) == 0)
        {
            if ((err = StopChunk(iff, types[type], ID_BODY)) == 0)
            {
                if ((err = StopOnExit(iff, types[type], ID_FORM)) == 0)
                {
                    if ((err = ParseIFF(iff, IFFPARSE_SCAN)) == 0 ||
                         err == IFFERR_EOC ||
                         err == IFFERR_EOF)
                    {
                        return(err);
                    }
                }
            }
        }
        CloseIFF(iff);
    }
    return(err);
}

/*
 * Retrieve stored property data.
 */

UBYTE *findProp(struct IFFHandle *iff, ULONG type, ULONG id)
{
    struct StoredProperty *sp;

    if (sp = FindProp(iff, type, id))
    {
        return(sp->sp_Data);
    }
    return(NULL);
}

/*
 * Buffered ReadChunkBytes.
 */

LONG readChunkBytes(struct bodyBuffer *body, BYTE *dest, WORD bytes)
{
    LONG leftBytes, sum = 0;

    while (bytes > 0)
    {
        if (body->left == 0)
        {
            if ((body->left = ReadChunkBytes(body->iff, body->buffer, BODYBUF_SIZE)) == 0)
            {
                return(sum);
            }
            body->current = body->buffer;
        }

        leftBytes = MIN(bytes, body->left);

        CopyMem(body->current, dest, leftBytes);
        body->current += leftBytes;
        dest += leftBytes;
        bytes -= leftBytes;
        body->left -= leftBytes;
        sum += leftBytes;
    }
    return(sum);
}

/*
 * Load color data.
 */

struct ColorMap *loadCMAP(struct IFFHandle *iff)
{
    struct StoredProperty *sp;
    struct ColorMap *cm;

    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        WORD colors = sp->sp_Size / 3;
        UBYTE *cmap = sp->sp_Data;

        if (cm = GetColorMap(colors))
        {
            WORD col;

            for (col = 0; col < colors; col++)
            {
                UBYTE red = *cmap++, green = *cmap++, blue = *cmap++;

                SetRGB32CM(cm, col, RGB(red), RGB(green), RGB(blue));
            }
            return(cm);
        }
    }
    return(NULL);
}

/*
 * Load bitmap data.
 */

BOOL unpackRow(UBYTE cmp, WORD bpr, struct bodyBuffer *body, BYTE **planePtr)
{
    BYTE *plane = *planePtr;

    if (cmp == cmpNone)
    {
        if (readChunkBytes(body, plane, bpr) != bpr)
        {
            return(FALSE);
        }
        plane += bpr;
    }
    else
    {
        while (bpr > 0)
        {
            BYTE con;
            if (readChunkBytes(body, &con, 1) != 1)
            {
                return(FALSE);
            }
            if (con >= 0)
            {
                WORD count = con + 1;
                if (readChunkBytes(body, plane, count) != count)
                {
                    return(FALSE);
                }
                plane += count;
                bpr -= count;
            }
            else if (con != -128)
            {
                WORD count = (-con) + 1;
                BYTE data;
                if (readChunkBytes(body, &data, 1) != 1)
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
    *planePtr = plane;
    return(TRUE);
}

struct BitMap *readILBM(struct IFFHandle *iff)
{
    BMHD bmhd = (BMHD)findProp(iff, ID_ILBM, ID_BMHD);
    WORD width, height;
    WORD bpr;
    UBYTE depth;
    UBYTE cmp, msk;
    struct BitMap *bm;
    struct bodyBuffer bodyBuf;
    WORD plane, row;
    PLANEPTR planes[9];
    BOOL success = FALSE;

    if (!bmhd)
    {
        return(NULL);
    }

    cmp = bmhd->bmh_Compression;
    msk = bmhd->bmh_Masking;

    if (cmp != cmpNone && cmp != cmpByteRun1)
    {
        return(NULL);
    }

    if (msk != mskNone && msk != mskHasTransparentColor && msk != mskHasMask)
    {
        return(NULL);
    }

    if (bodyBuf.buffer = bodyBuf.current = AllocMem(BODYBUF_SIZE, MEMF_PUBLIC))
    {
        bodyBuf.iff  = iff;
        bodyBuf.left = 0;

        width  = bmhd->bmh_Width;
        height = bmhd->bmh_Height;
        depth  = bmhd->bmh_Depth;

        bpr = RowBytes(width);

        if (msk == mskHasMask)
        {
            ++depth;
        }

        if (bm = AllocBitMap(width, height, depth, 0, NULL))
        {
            for (plane = 0; plane < depth; plane++)
            {
                planes[plane] = bm->Planes[plane];
            }

            for (row = 0; row < height; row++)
            {
                for (plane = 0; plane < depth; plane++)
                {
                    if (!(success = unpackRow(cmp, bpr, &bodyBuf, &planes[plane])))
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
                FreeMem(bodyBuf.buffer, BODYBUF_SIZE);
                return(bm);
            }
            FreeBitMap(bm);
        }
        FreeMem(bodyBuf.buffer, BODYBUF_SIZE);
    }
    return(NULL);
}

/*
 * Reading IFF from DOS streams.
 */

struct IFFHandle *openIFF(STRPTR name, ULONG type)
{
    struct IFFHandle *iff;

    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = Open(name, MODE_OLDFILE))
        {
            InitIFFasDOS(iff);
            if (scanIFF(iff, type) == 0)
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
