
#include <exec/types.h>
#include <graphics/displayinfo.h>

struct screenInfo
{
    struct DimensionInfo dims;
    ULONG *palette;
    struct BitMap *bm[ 2 ];
    struct Screen *s;
    struct DBufInfo *dbi;
};

BOOL openScreen(struct screenInfo *si, ULONG modeID, UBYTE depth);
void closeScreen(struct screenInfo *si);
