
/* $Id$ */

#ifndef SCREEN_H
#define SCREEN_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

struct copperData
{
    struct ViewPort *vp;
    WORD signal;
    struct Task *task;
};

struct screenData
{
    struct Screen *screen;
    struct ScreenBuffer *buffers[ 2 ];
    struct MsgPort *safePort;
    BOOL safeToDraw;
    UWORD toggleFrame;
    struct Interrupt *is;
    struct copperData cop;
    struct windowData *backwd;
    struct MsgPort *userPort;
    struct BitMap *gfx;
};

struct screenData *screenOpen( UBYTE colorDepth );
VOID screenClose( struct screenData *sd );

#endif /* SCREEN_H */
