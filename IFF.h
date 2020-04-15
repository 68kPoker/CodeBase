
/* IFF.h */

#include <exec/types.h>

struct scanInfo
{
    BOOL clip;
    ULONG type; /* Requested file type */
    ULONG *props; /* Requested properties */
    ULONG *stops; /* Requested stops */
};

/* For buffered reads */

struct buffer
{
    BYTE *buf; /* Beginning of buffer */
    LONG size; /* Total size of buffer */
    BYTE *end; /* Pointer to end of data */
    BYTE *cur; /* Current pointer */
};

/* Open and scan IFF */

struct IFFHandle *scanIFF(struct scanInfo *, STRPTR name);

void initBuffer(struct buffer *buffer);

/* Buffered ReadChunkBytes() */

LONG readChunkBytes(struct IFFHandle *, BYTE *, LONG count, struct buffer *);

/* Close opened IFF */

void closeIFF(struct IFFHandle *, struct scanInfo *);
