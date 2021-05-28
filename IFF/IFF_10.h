
/* $Id$ */

#ifndef IFF_H
#define IFF_H

#include <exec/types.h>

#define ID_BODY MAKE_ID('B','O','D','Y')
#define MIN(a,b) ((a)<(b)?(a):(b))

#define IFFBUF_SIZE 4096

struct IFFInfo
{
    BOOL clip;
    LONG err;
};

struct buffer
{
    BYTE *buffer, *current;
    LONG leftAmount, size;
};

struct IFFHandle *openIFF(STRPTR name, LONG mode, struct IFFInfo *ii);
void closeIFF(struct IFFHandle *iff, struct IFFInfo *ii);
LONG parseIFF(struct IFFHandle *iff, ULONG *props, ULONG type);

BOOL initBuffer(struct buffer *buf);
void freeBuffer(struct buffer *buf);
LONG readChunkBytes(struct IFFHandle *iff, struct buffer *buf, BYTE *dest, LONG amount);

#endif /* IFF_H */
