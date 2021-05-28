
/* Potrzebujë:
** - Funkcji do czytania obrazka do BitMapy i palety kolorów,
** - dúwiëku,
** - czytania i zapisu plansz.
*/

#include "IFF.h"

#include <datatypes/pictureclass.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/graphics_protos.h>

/* Praca na pliku IFF */
LONG workOnIFF(APTR user, STRPTR name, LONG dosmode, LONG mode, LONG (*readWriteIFF)(struct IFFHandle *iff, APTR user))
{
    struct IFFHandle *iff;
    LONG err = 1;
    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = Open(name, dosmode))
        {
            InitIFFasDOS(iff);
            if ((err = OpenIFF(iff, mode)) == 0)
            {
                err = readWriteIFF(iff, user);
                CloseIFF(iff);
            }
            Close(iff->iff_Stream);
        }
        FreeIFF(iff);
    }
    return(err);
}

/* Odczyt obrazka */
LONG readILBM(struct IFFHandle *iff, struct ILBMInfo *ii)
{
    LONG err;
    if ((err = PropChunk(iff, ID_ILBM, ID_BMHD)) == 0)
    {
        if ((err = PropChunk(iff, ID_ILBM, ID_CMAP)) == 0)
        {
            if ((err = StopChunk(iff, ID_ILBM, ID_BODY)) == 0)
            {
                if ((err = ParseIFF(iff, IFFPARSE_SCAN)) == 0)
                {
                    err = readBitMapColors(iff, ii);
                }
            }
        }
    }
    return(err);
}

LONG readBitMapColors(struct IFFHandle *iff, struct ILBMInfo *ii)
{
    LONG err;
    if ((err = readColors(iff, ii)) == 0)
    {
        if ((err = readBitMap(iff, ii)) == 0)
        {
            return(0);
        }
        freeColors(ii);
    }
    return(err);
}

/* Wczytanie kolorów */
LONG readColors(struct IFFHandle *iff, struct ILBMInfo *ii)
{
    struct StoredProperty *sp;
    if (sp = FindProp(iff, ID_ILBM, ID_CMAP))
    {
        ULONG size = sp->sp_Size;
        UWORD colors = size / 3;
        UWORD col;
        UBYTE *cmap = sp->sp_Data;
        struct ColorMap *cm;

        if (cm = GetColorMap(colors))
        {
            for (col = 0; col < colors; col++)
            {
                UBYTE red   = *cmap++;
                UBYTE green = *cmap++;
                UBYTE blue  = *cmap++;

                SetRGB32CM(cm, col, RGB(red), RGB(green), RGB(blue));
            }
            ii->cm = cm;
            return(0);
        }
    }
    return(1);
}

/* Wczytanie obrazka */
LONG readBitMap(struct IFFHandle *iff, struct ILBMInfo *ii)
{
    struct StoredProperty *sp;
    struct BitMapHeader *bmhd;
    struct BitMap *bm;
    UWORD w, h;
    UBYTE d, cmp, msk;
    UWORD bpr;
    PLANEPTR planes[9];
    UWORD row;
    UBYTE pl;
    LONG err = 1;
    struct IFFBuffer buf = { iff };

    if (sp = FindProp(iff, ID_ILBM, ID_BMHD))
    {
        bmhd = (struct BitMapHeader *)sp->sp_Data;
        w = bmhd->bmh_Width;
        h = bmhd->bmh_Height;
        d = bmhd->bmh_Depth;

        bpr = RowBytes(w);

        cmp = bmhd->bmh_Compression;
        msk = bmhd->bmh_Masking;

        if (bm = AllocBitMap(w, h, d, 0, NULL))
        {
            for (row = 0; row < h; row++)
            {
                for (pl = 0; pl < d; pl++)
                {
                    if ((err = unpackRow(&buf, planes[pl], bpr, cmp)) != 0)
                    {
                        break;
                    }
                    planes[pl] += bm->BytesPerRow;
                }
                if (err)
                {
                    break;
                }
            }
            if (buf.beg)
            {
                FreeMem(buf.beg, buf.size);
            }
            if (!err)
            {
                ii->bm = bm;
                return(0);
            }
            FreeBitMap(bm);
        }
    }
    return(err);
}

/* Rozpakowanie wiersza */
LONG unpackRow(struct IFFBuffer *buf, BYTE *plane, UWORD bpr, UBYTE cmp)
{
    if (cmp == cmpNone)
    {
        if (readChunkBytes(buf, plane, bpr) != bpr)
        {
            return(1);
        }
    }
    else if (cmp == cmpByteRun1)
    {
        while (bpr > 0)
        {
            BYTE con;
            if (readChunkBytes(buf, &con, 1) != 1)
            {
                return(1);
            }
            if (con >= 0)
            {
                WORD count = con + 1;
                if (bpr < count || readChunkBytes(buf, plane, count) != count)
                {
                    return(1);
                }
                bpr -= count;
                plane += count;
            }
            else if (con != -128)
            {
                WORD count = (-con) + 1;
                BYTE data;
                if (bpr < count || readChunkBytes(buf, &data, 1) != 1)
                {
                    return(1);
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
        return(1);
    }
    return(0);
}

/* Buforowany odczyt */
ULONG readChunkBytes(struct IFFBuffer *buf, BYTE *dest, ULONG size)
{
    ULONG actual = 0;
    ULONG min;

    if (!buf->beg)
    {
        if ((buf->beg = AllocMem(buf->size = SIZE, MEMF_PUBLIC)) == NULL)
        {
            return(0);
        }
    }

    while (size > 0)
    {
        if (buf->left == 0)
        {
            if ((buf->left = ReadChunkBytes(buf->iff, buf->beg, buf->size)) == 0)
            {
                break;
            }
            buf->cur = buf->beg;
        }

        if (size < buf->left)
        {
            min = size;
        }
        else
        {
            min = buf->left;
        }
        CopyMem(buf->cur, dest, min);
        actual   += min;
        buf->cur += min;
        dest     += min;

        buf->left -= min;
        size      -= min;
    }
    return(actual);
}
