
/* $Id$ */

#include <exec/types.h>

#define BODY_BUFFER_SIZE 1024

#define RowBytes( w ) ((((w) + 15) >> 4) << 1)
#define RGB( c ) ((c) | ((c)<<8) | ((c)<<16) | ((c)<<24))

struct ILBMInfo
{
    struct BitMapHeader *bmhd;
    struct BitMap *brush;
    ULONG *palette;
};

struct bodyBuffer
{
    BYTE *beg, *cur;
    LONG size, left;
};
