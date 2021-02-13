
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/interrupts.h>

struct screenInfo
{
    struct Screen *screen;
    struct BitMap *scrBitMaps[2];
    struct DBufInfo *dbufInfo;
    BOOL safeToWrite;
    WORD toggleFrame;
    struct MsgPort *safePort;
    struct Interrupt copperIs;
    struct copperInfo
    {
        WORD signal;
        struct Task *task;
        struct ViewPort *viewPort;
    } copper;
    struct Region *syncRegion[2];
    struct Rectangle dClip;
    struct BitMap *gfxBitMap;
};

ULONG obtainModeID(UWORD width, UWORD height, UBYTE depth);
struct Screen *openScreen(STRPTR title, ULONG modeID, UBYTE depth, struct screenInfo *scrInfo);
BOOL addCopper(struct screenInfo *scrInfo, UBYTE pri);
BOOL addRegions(struct screenInfo *scrInfo);
VOID syncScreen(struct screenInfo *scrInfo);
BOOL remRegions(struct screenInfo *scrInfo);
VOID remCopper(struct screenInfo *scrInfo);
VOID closeScreen(struct Screen *screen);

#endif /* SCREEN_H */
