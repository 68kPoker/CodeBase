
#ifndef SCREEN_H
#define SCREEN_H

#include <exec/types.h>

typedef struct Screen SCR;

typedef struct screenInfo
{
    struct Screen *screen;
    struct BitMap *bitmaps[2];
    struct ScreenBuffer *buffers[2];
    struct MsgPort *safeport;
    BOOL safe;
    WORD frame;
} SCRINFO;

#endif /* SCREEN_H */
