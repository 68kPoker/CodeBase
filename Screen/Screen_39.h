
/* $Id$ */

#include <exec/types.h>

/* Additional data */

struct screenInfo
{
    struct BitMap       *bitmaps[ 2 ];
    struct Screen       *screen;
    struct ScreenBuffer *scrBuffers[ 2 ];
    struct MsgPort      *safePort;
    BOOL                safeToDraw;
    WORD                curBuffer;
    struct Window       *window;
};
