
#include <exec/types.h>

struct screenInfo
{
    ULONG *pal;
    struct ScreenBuffer *sb[2];
    struct Window *w;
};

BOOL allocBitMaps(struct BitMap *bitmaps[], ULONG modeID, UBYTE depth);
void freeBitMaps(struct BitMap *bitmaps[]);

BOOL getColors(struct screenInfo *si, struct ColorMap *cm);
void freeColors(struct screenInfo *si);

struct Window *openWindow(struct screenInfo *si, struct BitMap *bitmaps[]);
void closeWindow(struct Window *w, struct screenInfo *si);
