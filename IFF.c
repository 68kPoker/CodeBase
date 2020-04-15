
#include <dos/dos.h>
#include <libraries/iffparse.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>

#include "IFF.h"

LONG countChunks(ULONG *chks)
{
    LONG n = 0;
    if (chks == NULL)
        return(0);

    while (*chks++)
        n++;
    return(n / 2);
}

/* Open and scan IFF */

struct IFFHandle *scanIFF(struct scanInfo *si, STRPTR name)
{
    struct IFFHandle *iff;

    if (iff = AllocIFF())
    {
        if (iff->iff_Stream = Open(name, MODE_OLDFILE))
        {
            InitIFFasDOS(iff);
            if (OpenIFF(iff, IFFF_READ) == 0)
            {
                if (PropChunks(iff, si->props, countChunks(si->props)) == 0)
                {
                    if (StopChunks(iff, si->stops, countChunks(si->stops)) == 0)
                    {
                        if (ParseIFF(iff, IFFPARSE_SCAN) == 0)
                        {
                            return(iff);
                        }
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

void initBuffer(struct buffer *buffer)
{
    buffer->cur = buffer->end = buffer->buf + buffer->size;
}

/* Buffered ReadChunkBytes() */

LONG readChunkBytes(struct IFFHandle *iff, BYTE *buf, LONG count, struct buffer *buffer)
{
    BYTE *beg = buffer->buf;
    BYTE *cur = buffer->cur;
    BYTE *end = buffer->end;
    LONG size = buffer->size;

    LONG left = end - cur;

    while (count > left)
    {
        if (left > 0)
        {
            /* Copy what's left */
            CopyMem(cur, buf, left);
            count -= left;
        }

        /* Set current at beginning of buffer */
        cur = beg;

        WORD read;
        read = ReadChunkBytes(iff, cur, size);

        end = beg + read;

        if (read < size)
        {
            /* Last read */
            if (read < count)
                /* Data underflow */
                return(-1);
        }

        left = end - cur;
    }

    if (count > 0)
    {
        /* Copy last chunk */
        CopyMem(cur, buf, count);
        cur += count;
    }

    buffer->cur = cur;
    buffer->end = end;
    return(0);
}

/* Close opened IFF */

void closeIFF(struct IFFHandle *iff, struct scanInfo *si)
{
    CloseIFF(iff);
    Close(iff->iff_Stream);
    FreeIFF(iff);
}
