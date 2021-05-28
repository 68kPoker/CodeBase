
#include <exec/types.h>

#include "Picture.h"

struct ScreenInfo
    {
    struct BitMap *bitmaps[2];
    struct Screen *screen;
    struct ScreenBuffer *buffers[2];
    struct MsgPort *ports[2];
    BOOL safe[2];
    WORD frame;
    };

LONG allocScreenBitMaps(struct ScreenInfo *si, UWORD width, UWORD height, UBYTE depth);
VOID freeScreenBitMaps(struct ScreenInfo *si);

LONG openScreen(struct ScreenInfo *si, struct PictureInfo *pi);
VOID closeScreen(struct ScreenInfo *si);
