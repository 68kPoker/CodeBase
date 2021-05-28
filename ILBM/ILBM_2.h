
#ifndef ILBM_H
#define ILBM_H

#include "IFF.h"
#include <exec/types.h>

#define RowBytes(w) ((((w)+15)>>4)<<1)
#define RGB(c) ((c)|((c)<<8)|((c)<<16)|((c)<<24))

typedef struct infoILBM
{
    IFF iff;
    struct BitMapHeader *bmhd;
    struct BitMap *bm;
    struct ColorMap *cm;
    BOOL mask;
} ILBM;

BOOL queryILBM(ILBM *ilbm, STRPTR name);
BOOL loadCMAP(ILBM *ilbm);
BOOL loadILBM(ILBM *ilbm);
VOID freeILBM(ILBM *ilbm);

#endif /* ILBM_H */
