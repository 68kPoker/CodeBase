
/* $Id$ */

#include <stdlib.h>
#include <stdio.h>
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>

#include "IFF.h"

/* Otwórz plik/schowek IFF do odczytu/zapisu */
struct IFFHandle *openIFF(STRPTR name, LONG mode, struct IFFInfo *ii)
{
    struct IFFHandle *iff;
    if (iff = AllocIFF())
    {
        BOOL clip = FALSE;
        ULONG s;
        LONG dosmode = (mode == IFFF_WRITE ? MODE_NEWFILE : MODE_OLDFILE);
        UBYTE unit;

        if (name[0] == '-' && name[1] == 'c')
        {
            clip = TRUE;
            unit = name[2] ? atoi(name + 2) : PRIMARY_CLIP;
        }

        if (s = clip ? (ULONG)OpenClipboard(unit) : (ULONG)Open(name, dosmode))
        {
            LONG err;
            iff->iff_Stream = s;
            clip ? InitIFFasClip(iff) : InitIFFasDOS(iff);
            if ((err = OpenIFF(iff, mode)) == 0)
            {
                ii->clip = clip;
                ii->err  = err;
                return(iff);
            }
            clip ? CloseClipboard((struct ClipboardHandle *)s) : Close((BPTR)s);
        }
        else if (clip)
            printf("Couldn't open clipboard %d\n", unit);
        else
            printf("Couldn't open %s\n", name);
        FreeIFF(iff);
    }
    return(NULL);
}

/* Zamknij plik IFF */
void closeIFF(struct IFFHandle *iff, struct IFFInfo *ii)
{
    ULONG s = iff->iff_Stream;

    CloseIFF(iff);
    ii->clip ? CloseClipboard((struct ClipboardHandle *)s) : Close((BPTR)s);
    FreeIFF(iff);
}

/* Zlicz pary identyfikatorów w tablicy */
LONG countChunks(ULONG *props)
{
    LONG count = 0;

    if (!props)
    {
        return(0);
    }
    while (*props++)
    {
        count++;
    }
    return(count / 2);
}

/* Zainstaluj chunki typowe dla danego pliku IFF, przeskanuj plik */
LONG parseIFF(struct IFFHandle *iff, ULONG *props, ULONG type)
{
    LONG err;
    if ((err = PropChunks(iff, props, countChunks(props))) == 0)
    {
        if ((err = StopChunk(iff, type, ID_BODY)) == 0)
        {
            if ((err = ParseIFF(iff, IFFPARSE_SCAN)) == 0 ||
                err == IFFERR_EOC ||
                err == IFFERR_EOF)
            {
                return(err);
            }
        }
    }
    return(err);
}

/* Przygotuj bufor */
BOOL initBuffer(struct buffer *buf)
{
    if (buf->buffer = buf->current = AllocMem(IFFBUF_SIZE, MEMF_PUBLIC))
    {
        buf->leftAmount = 0;
        buf->size = IFFBUF_SIZE;
        return(TRUE);
    }
    return(FALSE);
}

/* Zwolnij bufor */
void freeBuffer(struct buffer *buf)
{
    FreeMem(buf->buffer, buf->size);
}

/* Buforowany odczyt z plików IFF */
LONG readChunkBytes(struct IFFHandle *iff, struct buffer *buf, BYTE *dest, LONG amount)
{
    LONG sum = 0;
    LONG bytes;

    while (amount > 0)
    {
        if (buf->leftAmount == 0)
        {
            if ((buf->leftAmount = ReadChunkBytes(iff, buf->buffer, buf->size)) == 0)
            {
                return(sum);
            }
            buf->current = buf->buffer;
        }
        bytes = MIN(amount, buf->leftAmount);
        CopyMem(buf->current, dest, bytes);
        buf->current += bytes;
        dest += bytes;
        sum += bytes;
        amount -= bytes;
        buf->leftAmount -= bytes;
    }
    return(sum);
}
