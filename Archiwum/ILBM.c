
#include <datatypes/pictureclass.h>

#include <clib/iffparse_protos.h>
#include <clib/exec_protos.h>

struct IFFBuffer
{
    struct IFFHandle *iff;
    BYTE *buf, *cur;
    LONG size, bytesLeft;
};

LONG readChunkBytes(struct IFFBuffer *buf, BYTE *dest, UWORD bytesToCopy)
{
    LONG copied = 0, minBytes;

    while (bytesToCopy > 0)
    {
        if (buf->bytesLeft == 0)
        {
            if ((buf->bytesLeft = ReadChunkBytes(buf->iff, buf->buf, buf->size)) == 0)
                break;
            buf->cur = buf->buf;
        }

        if (bytesToCopy < buf->bytesLeft)
            minBytes = bytesToCopy;
        else
            minBytes = buf->bytesLeft;

        CopyMem(buf->cur, dest, minBytes);

        bytesToCopy -= minBytes;
        buf->bytesLeft -= minBytes;

        copied += minBytes;
        dest += minBytes;
        buf->cur += minBytes;
    }
    return copied;
}

BOOL copyChunkBytes(struct IFFBuffer *buf, BYTE **dest, UWORD count, UWORD *bpr)
{
    if (*bpr < count)
        return FALSE;

    if (readChunkBytes(buf, *dest, count) != count)
        return FALSE;

    *dest += count;
    *bpr -= count;

    return TRUE;
}

BOOL repeatChunkByte(struct IFFBuffer *buf, BYTE **dest, UWORD count, UWORD *bpr)
{
    BYTE data;
    BYTE *ptr = *dest;

    if (*bpr < count)
        return FALSE;

    if (readChunkBytes(buf, &data, 1) != 1)
        return FALSE;

    while (count-- > 0)
        *ptr++ = data;

    *dest = ptr;
    *bpr -= count;

    return TRUE;
}

BOOL unpackRow(struct IFFBuffer *buf, PLANEPTR plane, UBYTE cmp, UWORD bpr)
{
    switch (cmp)
    {
        case cmpNone:
            return readChunkBytes(buf, plane, bpr) == bpr;

        case cmpByteRun1:
            while (bpr > 0)
            {
                BYTE c;
                if (readChunkBytes(buf, &c, 1) != 1)
                    return FALSE;

                if (c >= 0)
                {
                    if (!copyChunkBytes(buf, &plane, c + 1, &bpr))
                        return FALSE;
                }
                else if (c != -128)
                {
                    if (!repeatChunkByte(buf, &plane, (-c) + 1, &bpr))
                        return FALSE;
                }
            }
            return TRUE;

        default:
            return FALSE;
    }
}
