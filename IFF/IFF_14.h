
#include <exec/types.h>

struct ILBMInfo
    {
    struct BitMap *bm;
    struct ColorMap *cm;
    };

BOOL loadILBM(struct ILBMInfo *ii, STRPTR name);
VOID freeILBM(struct ILBMInfo *ii);
