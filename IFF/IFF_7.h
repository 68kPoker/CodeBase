
#ifndef IFF_H
#define IFF_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

#define RowBytes(w) ((((w)+15)>>4)<<1)

struct IFFInfo
{
    struct IFFHandle *iff;
    BOOL clip;
    LONG err;
    BYTE *buffer;
    LONG size;
};

struct ILBMInfo
{
    struct IFFInfo ii;
    struct BitMapHeader *bmhd;
    ULONG *colors;
    struct BitMap *bm;
};

BOOL loadILBM(struct ILBMInfo *ilbm, STRPTR name);
VOID unloadILBM(struct ILBMInfo *ilbm);

#endif /* IFF_H */
